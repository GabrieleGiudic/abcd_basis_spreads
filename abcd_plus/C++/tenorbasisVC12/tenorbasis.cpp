/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015, 2016 Ferdinando Ametrano
 Copyright (C) 2015, 2016 Paolo Mazzocchi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/experimental/tenorbasis/tenorbasis.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/math/abcdmathfunction.hpp>

using boost::shared_ptr;
using std::vector;

namespace QuantLib {

	namespace {
		void no_deletion(TenorBasis*) {}
	}


	TenorBasis::TenorBasis(Size nArguments,
		shared_ptr<IborIndex> iborIndex,
		boost::shared_ptr<IborIndex> baseIborIndex,
		Date referenceDate)
		: CalibratedModel(nArguments),
		iborIndex_(iborIndex), baseIborIndex_(baseIborIndex),
		referenceDate_(referenceDate),
		// Actual365Fixed is hardcoded: it must be an invertible DayCounter.
		// If changed, TenorBasis::dateFromTime(Time) must be updated
		dateTimeDC_(Actual365Fixed()), sign_(1.0) {

		QL_REQUIRE(iborIndex != 0, "empty iborIndex");
		if (referenceDate_ == Date()) {
			Date today = Settings::instance().evaluationDate();
			referenceDate_ = iborIndex->valueDate(today);
			// avoid the following as the Hande could be empty
			//referenceDate_ = baseCurve_->referenceDate();
		}
		dc_ = iborIndex_->dayCounter();
		bdc_ = iborIndex_->businessDayConvention();
		eom_ = iborIndex_->endOfMonth();
		cal_ = iborIndex_->fixingCalendar();
		tenor_ = iborIndex_->tenor();
		Date endDate = cal_.advance(referenceDate_, tenor_, bdc_, eom_);
		tau_ = dc_.yearFraction(referenceDate_, endDate);

		QL_REQUIRE(baseIborIndex != 0, "empty base iborIndex");
		if (baseIborIndex_->tenor() > tenor_)
			sign_ = -1.0;
		registerWith(baseIborIndex_);
	}

	Real TenorBasis::value(

		const Array& params,
		const vector<boost::shared_ptr<RateHelper> >& h) {

		std::vector<boost::shared_ptr<CalibrationHelperBase> >
			cH(h.size());

		for (Size i = 0; i < h.size(); ++i) {
			cH[i] = h[i];
		}
		Real value;
		return value = CalibratedModel::value(params, cH);

	}

	Spread TenorBasis::value(Date d) const {
		Time t = timeFromReference(d);
		return value(t);
	}

	Spread TenorBasis::instBasisValue(Date d) const {
		Time t = timeFromReference(d);
		return instBasisValue(t);
	}

	Rate TenorBasis::tenorForwardRate(Date d1) const {
		Date d2 = cal_.advance(d1, tenor_, bdc_, eom_);
		// baseCurve must be a discounting curve...
		// otherwise it could not provide fwd(d1, d2) with d2-d1!=tau
		Handle<YieldTermStructure> baseCurve =
			baseIborIndex_->forwardingTermStructure();
		Real accrFactor = baseCurve->discount(d1) / baseCurve->discount(d2);
		Time dt = dc_.yearFraction(d1, d2);
		Rate baseFwd = (accrFactor - 1.0) / dt;

		Rate basis = value(d1);
		return baseFwd + sign_ * basis;
	}

	Rate TenorBasis::tenorForwardRate(Time t1) const {
		// we need Date algebra to calculate d2
		Date d1 = dateFromTime(t1);
		return tenorForwardRate(d1);
	}

	Rate TenorBasis::accrualFactor(Date d1,
		Date d2) const {
		QL_REQUIRE(d1 <= d2,
			"d2 (" << d2 << ") cannot be before d1 (" << d1 << ")");
		// baseCurve must be a discounting curve...
		// otherwise it could not provide fwd(t1, t2) with t2-t1!=tau_
		Handle<YieldTermStructure> baseCurve =
			baseIborIndex_->forwardingTermStructure();
		Real accrFactor = baseCurve->discount(d1) / baseCurve->discount(d2);
		Real instContBasisIntegral = sign_ * integrate_(d1, d2);
		accrFactor *= std::exp(instContBasisIntegral);
		return accrFactor;
	}

	Rate TenorBasis::accrualFactor(Time t1,
		Time t2) const {
		Date d1 = dateFromTime(t1);
		Date d2 = dateFromTime(t2);
		return accrualFactor(d1, d2);
	}

	Rate TenorBasis::forwardRate(Date d1) const {
		Date d2 = cal_.advance(d1, tenor_, bdc_, eom_);
		return forwardRate(d1, d2);
	}

	Rate TenorBasis::forwardRate(Date d1,
		Date d2) const {
		Real accrFactor = accrualFactor(d1, d2);
		Time dt = dc_.yearFraction(d1, d2);
		Rate fwd = (accrFactor - 1.0) / dt;
		return fwd;
	}

	Rate TenorBasis::forwardRate(Time t1,
		Time t2) const {
		Date d1 = dateFromTime(t1);
		Date d2 = dateFromTime(t2);
		return forwardRate(d1, d2);
	}

	Time TenorBasis::timeFromReference(Date d) const {
		// Actual365Fixed is hardcoded at construction time.
		// see also TenorBasis::dateFromTime(Time t)
		return dateTimeDC_.yearFraction(referenceDate_, d);
	}

	Date TenorBasis::dateFromTime(Time t) const {
		// Actual365Fixed is hardcoded at construction time.
		// It must be an invertible DayCounter,
		// see also TenorBasis::timeFromReference(Date d)
		BigInteger days = BigInteger(t*365.0 + 0.5);
		BigInteger result = referenceDate_.serialNumber() + days;
		if (result >= Date::maxDate().serialNumber())
			return Date::maxDate();
		return Date(result);
	}

	const shared_ptr<IborIndex>& TenorBasis::iborIndex() const {
		return iborIndex_;
	}

	const boost::shared_ptr<IborIndex>& TenorBasis::baseIborIndex() const {
		return baseIborIndex_;
	}

	Constraint TenorBasis::constraint() const {
		return NoConstraint();
	}

	Real TenorBasis::integrate_(Date d1) const {
		Date d2 = cal_.advance(d1, tenor_, bdc_, eom_);
		return integrate_(d1, d2);
	}

	Real TenorBasis::integrate_(Date d1,
		Date d2) const {
		Time t1 = timeFromReference(d1);
		Time t2 = timeFromReference(d2);
		return integrate_(t1, t2);
	}

	void TenorBasis::calibrate(
		const std::vector<boost::shared_ptr<RateHelper> >& helpers,
		OptimizationMethod& method,
		const EndCriteria& endCriteria,
		const std::vector<Real>& weights,
		const std::vector<bool>& fixParameters) {
		TenorBasisYieldTermStructure
			yts(boost::shared_ptr<TenorBasis>(this, no_deletion));
		std::vector<boost::shared_ptr<CalibrationHelperBase> >
			cHelpers(helpers.size());

		for (Size i = 0; i < helpers.size(); ++i) {
			helpers[i]->setTermStructure(&yts);
			cHelpers[i] = helpers[i];
		}
		CalibratedModel::calibrate(cHelpers, method, endCriteria,
			constraint(), weights, fixParameters);
	}

	void TenorBasis::forwardCalibrate(
		const std::vector<boost::shared_ptr<ForwardHelper> >& helpers,
		OptimizationMethod& method,
		const EndCriteria& endCriteria,
		const std::vector<Real>& weights,
		const std::vector<bool>& fixParameters) {
		TenorBasisForwardRateCurve
			yts(boost::shared_ptr<TenorBasis>(this, no_deletion));
		std::vector<boost::shared_ptr<CalibrationHelperBase> >
			cHelpers(helpers.size());
		for (Size i = 0; i < helpers.size(); ++i) {
			helpers[i]->setTermStructure(&yts);
			cHelpers[i] = helpers[i];
		}
		CalibratedModel::calibrate(cHelpers, method, endCriteria,
			constraint(), weights, fixParameters);
	}

	namespace {

		// to constrained <- from unconstrained
		std::vector<Real> direct(const std::vector<Real>& x) {
			std::vector<Real> y(4);
			y[2] = std::exp(x[2]);
			y[3] = std::exp(-x[3] * x[3]);
			y[0] = x[0] * x[0] - y[3];
			y[1] = x[1] * x[1];
			return y;
		}

		// to unconstrained <- from constrained
		std::vector<Real> inverse(const std::vector<Real>& x) {
			std::vector<Real> y(4);
			y[2] = std::log(x[2]);
			y[3] = std::sqrt((x[3] == 0 ? -std::log(QL_EPSILON)
				: -std::log(x[3])));
			y[0] = std::sqrt(x[0] + x[3]);
			y[1] = std::sqrt(x[1]);
			return y;
		}

		class AbcdConstraint : public Constraint {
		private:
			class Impl : public Constraint::Impl {
				Real tau_;
				bool isSimple_;
			public:
				Impl(Real tau, bool isSimple)
					: tau_(tau), isSimple_(isSimple) {}
				bool test(const Array& params) const {
					Real a = params[0];
					Real b = params[1];
					Real c = params[2];
					Real d = params[3];

					try {
						AbcdMathFunction::validate(a, b, c, d);
						AbcdMathFunction f(a, b, c, d);
						vector<Real> v;
						if (isSimple_) {
							v = f.definiteDerivativeCoefficients(0.0, tau_);
							AbcdMathFunction::validate(v[0] * tau_,
								v[1] * tau_,
								v[2],
								v[3] * tau_);
						}
						else {
							v = f.definiteIntegralCoefficients(0.0, tau_);
							AbcdMathFunction::validate(a / tau_,
								b / tau_,
								c,
								d / tau_);
						}
						return true;
					}
					catch (...) {
						return false;
					}
				}
			};
		public:
			AbcdConstraint(Real tau, bool isSimple)
				: Constraint(boost::shared_ptr<Constraint::Impl>(
					new AbcdConstraint::Impl(tau, isSimple))) {}
		};

	}

	AbcdTenorBasis::AbcdTenorBasis(shared_ptr<IborIndex> iborIndex,
		boost::shared_ptr<IborIndex> baseIborIndex,
		Date referenceDate,
		bool isSimple,
		const std::vector<Real>& coeff)
		: TenorBasis(4, iborIndex, baseIborIndex, referenceDate) {
		//std::vector<Real> y = inverse(coeff);
		std::vector<Real> y = coeff;
		arguments_[0] = ConstantParameter(y[0], NoConstraint());
		arguments_[1] = ConstantParameter(y[1], NoConstraint());
		arguments_[2] = ConstantParameter(y[2], NoConstraint());
		arguments_[3] = ConstantParameter(y[3], NoConstraint());
		isSimple_ = isSimple;
		generateArguments();
	}

	Spread AbcdTenorBasis::value(Time t)const { return (*basis_)(t); };

	Constraint AbcdTenorBasis::constraint() const {
		return AbcdConstraint(tau_, isSimple_);
	}

	void AbcdTenorBasis::generateArguments() {
		std::vector<Real> x(4);
		x[0] = arguments_[0](0.0);
		x[1] = arguments_[1](0.0);
		x[2] = arguments_[2](0.0);
		x[3] = arguments_[3](0.0);
		//std::vector<Real> y = direct(x);
		std::vector<Real> y = x;
		if (isSimple_) {
			basis_ = shared_ptr<AbcdMathFunction>(
				new AbcdMathFunction(y[0], y[1], y[2], y[3]));
			vector<Real> c = basis_->definiteDerivativeCoefficients(0.0, tau_);
			c[0] *= tau_;
			c[1] *= tau_;
			// unaltered c[2] (the c in abcd)
			c[3] *= tau_;
			instBasis_ = shared_ptr<AbcdMathFunction>(new AbcdMathFunction(c));
		}
		else {
			instBasis_ = shared_ptr<AbcdMathFunction>(
				new AbcdMathFunction(y[0], y[1], y[2], y[3]));
			vector<Real> c =
				instBasis_->definiteIntegralCoefficients(0.0, tau_);
			c[0] /= tau_;
			c[1] /= tau_;
			// unaltered c[2] (the c in abcd)
			c[3] /= tau_;
			basis_ = shared_ptr<AbcdMathFunction>(new AbcdMathFunction(c));
		}
	}

	const vector<Real>& AbcdTenorBasis::coefficients() const {
		return basis_->coefficients();
	}

	const vector<Real>& AbcdTenorBasis::instCoefficients() const {
		return instBasis_->coefficients();
	}

	Date AbcdTenorBasis::maximumLocation() const {
		Time maximumLocation = basis_->maximumLocation();
		return dateFromTime(maximumLocation);
	}

	Spread AbcdTenorBasis::maximumValue() const {
		Date d = maximumLocation();
		return TenorBasis::value(d);
	}

	Date AbcdTenorBasis::instBasisMaximumLocation() const {
		Time maximumLocation = instBasis_->maximumLocation();
		return dateFromTime(maximumLocation);
	}

	Spread AbcdTenorBasis::instBasisMaximumValue() const {
		Date d = maximumLocation();
		return TenorBasis::instBasisValue(d);
	}

	Real AbcdTenorBasis::integrate_(Time t1,
		Time t2) const {
		return instBasis_->definiteIntegral(t1, t2);
	}


	PolynomialTenorBasis::PolynomialTenorBasis(
		shared_ptr<IborIndex> iborIndex,
		shared_ptr<IborIndex> baseIborIndex,
		Date referenceDate,
		bool isSimple,
		const std::vector<Real>& coeff)
		: TenorBasis(coeff.size(), iborIndex, baseIborIndex, referenceDate),
		isSimple_(isSimple) {
		for (Size i = 0; i < coeff.size(); ++i)
			arguments_[i] = ConstantParameter(coeff[i], NoConstraint());
		generateArguments();
	}

	void PolynomialTenorBasis::generateArguments() {
		std::vector<Real> coeffs(arguments_.size());
		for (Size i = 0; i < coeffs.size(); ++i)
			coeffs[i] = arguments_[i](0.0);
		if (isSimple_) {
			basis_ =
				shared_ptr<PolynomialFunction>(new PolynomialFunction(coeffs));
			std::vector<Real> c =
				basis_->definiteDerivativeCoefficients(0.0, tau_);
			for (Size i = 0; i < c.size(); ++i)
				c[i] *= tau_;
			instBasis_ =
				shared_ptr<PolynomialFunction>(new PolynomialFunction(c));
		}
		else {
			instBasis_ =
				shared_ptr<PolynomialFunction>(new PolynomialFunction(coeffs));
			std::vector<Real> c =
				instBasis_->definiteIntegralCoefficients(0.0, tau_);
			for (Size i = 0; i < c.size(); ++i)
				c[i] /= tau_;
			basis_ =
				shared_ptr<PolynomialFunction>(new PolynomialFunction(c));
		}
	}

	const vector<Real>& PolynomialTenorBasis::coefficients() const {
		return basis_->coefficients();
	}

	const vector<Real>& PolynomialTenorBasis::instCoefficients() const {
		return instBasis_->coefficients();
	}

	Real PolynomialTenorBasis::integrate_(Time t1,
		Time t2) const {
		return instBasis_->definiteIntegral(t1, t2);
	}


	TenorBasisYieldTermStructure::TenorBasisYieldTermStructure(
		const boost::shared_ptr<TenorBasis>& basis)
		: YieldTermStructure(basis->dayCounter()), basis_(basis) {}

	const Date& TenorBasisYieldTermStructure::referenceDate() const {
		return basis_->referenceDate();
	}

	Calendar TenorBasisYieldTermStructure::calendar() const {
		return basis_->iborIndex()->fixingCalendar();
	}

	Natural TenorBasisYieldTermStructure::settlementDays() const {
		return basis_->iborIndex()->fixingDays();
	}

	Date TenorBasisYieldTermStructure::maxDate() const {
		return basis_->baseIborIndex()->forwardingTermStructure()->maxDate();
	}

	DiscountFactor TenorBasisYieldTermStructure::discountImpl(Time t) const {
		Date ref = referenceDate();
		Date d = basis_->dateFromTime(t);
		Real accrFactor = basis_->accrualFactor(ref, d);
		return 1.0 / accrFactor;
	}

	TenorBasisForwardRateCurve::TenorBasisForwardRateCurve(
		const boost::shared_ptr<TenorBasis>& basis)
		: ForwardRateCurve(basis->iborIndex()->familyName(),
			basis->iborIndex()->tenor(),
			basis->iborIndex()->fixingDays(),
			basis->iborIndex()->currency(),
			basis->iborIndex()->fixingCalendar(),
			basis->iborIndex()->businessDayConvention(),
			basis->iborIndex()->endOfMonth(),
			basis->iborIndex()->dayCounter(),
			basis->dayCounter()),
		basis_(basis) {}

	const Date& TenorBasisForwardRateCurve::referenceDate() const {
		return basis_->referenceDate();
	}

	Calendar TenorBasisForwardRateCurve::calendar() const {
		return basis_->iborIndex()->fixingCalendar();
	}

	Natural TenorBasisForwardRateCurve::settlementDays() const {
		return basis_->iborIndex()->fixingDays();
	}

	Date TenorBasisForwardRateCurve::maxDate() const {
		Handle<YieldTermStructure> baseCurve =
			basis_->baseIborIndex()->forwardingTermStructure();
		return baseCurve->maxDate();
	}

	Rate TenorBasisForwardRateCurve::forwardRate(
		Time t, bool extrapolate) const {
		return basis_->tenorForwardRate(t);
	}

	DiscountCorrectedTermStructure::DiscountCorrectedTermStructure(
		const Handle<YieldTermStructure>& bestFitCurve,
		const std::vector<boost::shared_ptr<RateHelper> >& instruments,
		Real accuracy)
		: bestFitCurve_(bestFitCurve), instruments_(instruments), accuracy_(accuracy) {
		registerWith(bestFitCurve_);
		bootstrap_.setup(this);
	}

	const Date& DiscountCorrectedTermStructure::referenceDate() const {
		return bestFitCurve_->referenceDate();
	}

	DayCounter DiscountCorrectedTermStructure::dayCounter() const {
		return bestFitCurve_->dayCounter();
	}

	Calendar DiscountCorrectedTermStructure::calendar() const {
		return bestFitCurve_->calendar();
	}

	Natural DiscountCorrectedTermStructure::settlementDays() const {
		return bestFitCurve_->settlementDays();
	}

	Date DiscountCorrectedTermStructure::maxDate() const {
		return bestFitCurve_->maxDate();
	}

	const std::vector<Time>& DiscountCorrectedTermStructure::times() const {
		calculate();
		return times_;
	}

	const std::vector<Date>& DiscountCorrectedTermStructure::dates() const {
		calculate();
		return dates_;
	}

	const std::vector<Real>& DiscountCorrectedTermStructure::data() const {
		calculate();
		return data_;
	}

	void DiscountCorrectedTermStructure::update() {
		LazyObject::update();
	}

	DiscountFactor DiscountCorrectedTermStructure::discountImpl(Time t) const {
		calculate();
		DiscountFactor d = bestFitCurve_->discount(t, true);
		Real k = interpolation_(t, true);
		return k * d;
	}

	void DiscountCorrectedTermStructure::performCalculations() const {
		bootstrap_.calculate();
	}

	ForwardCorrectedTermStructure::ForwardCorrectedTermStructure(
		const std::string& fwdFamilyName,
		const Period& fwdTenor,
		Natural fwdSettlementDays,
		const Currency& fwdCurrency,
		const Calendar& fwdFixingCalendar,
		BusinessDayConvention fwdConvention,
		bool fwdEndOfMonth,
		const DayCounter& fwdDayCounter,
		const Handle<ForwardRateCurve>& bestFitCurve,
		const std::vector<boost::shared_ptr<ForwardHelper> >& instruments,
		Real accuracy)
		: ForwardRateCurve(fwdFamilyName, fwdTenor, fwdSettlementDays, fwdCurrency,
			fwdFixingCalendar, fwdConvention, fwdEndOfMonth,
			fwdDayCounter),
		bestFitCurve_(bestFitCurve), instruments_(instruments), accuracy_(accuracy) {
		registerWith(bestFitCurve_);
		bootstrap_.setup(this);
	}

	const Date& ForwardCorrectedTermStructure::referenceDate() const {
		return bestFitCurve_->referenceDate();
	}

	DayCounter ForwardCorrectedTermStructure::dayCounter() const {
		return bestFitCurve_->dayCounter();
	}

	Calendar ForwardCorrectedTermStructure::calendar() const {
		return bestFitCurve_->calendar();
	}

	Natural ForwardCorrectedTermStructure::settlementDays() const {
		return bestFitCurve_->settlementDays();
	}

	Date ForwardCorrectedTermStructure::maxDate() const {
		return bestFitCurve_->maxDate();
	}

	const std::vector<Time>& ForwardCorrectedTermStructure::times() const {
		calculate();
		return times_;
	}

	const std::vector<Date>& ForwardCorrectedTermStructure::dates() const {
		calculate();
		return dates_;
	}

	const std::vector<Real>& ForwardCorrectedTermStructure::data() const {
		calculate();
		return data_;
	}

	void ForwardCorrectedTermStructure::update() {
		LazyObject::update();
	}

	Rate ForwardCorrectedTermStructure::forwardRate(Time t,
		bool extrapolate) const {
		calculate();
		Rate F = bestFitCurve_->forwardRate(t, true);
		Real k = interpolation_(t, true);
		return k * F;
	}

	void ForwardCorrectedTermStructure::performCalculations() const {
		bootstrap_.calculate();
	}

	AcdtTenorBasis::AcdtTenorBasis(boost::shared_ptr<IborIndex> iborIndex,
		boost::shared_ptr<IborIndex> baseIborIndex,
		Date referenceDate,
		bool isSimple,
		const std::vector<Real>& coeff)
		: AbcdTenorBasis(iborIndex, baseIborIndex, referenceDate, isSimple, coeffAbcd(coeff)) {
		//std::vector<Real> y = inverse(coeff);
		std::vector<Real> y = coeff;

		arguments_[0] = ConstantParameter(y[0], NoConstraint());
		arguments_[1] = ConstantParameter(y[1], NoConstraint());
		arguments_[2] = ConstantParameter(y[2], NoConstraint());
		arguments_[3] = ConstantParameter(y[3], NoConstraint()); //t_max
		isSimple_ = isSimple;
		generateArguments();
	}

	//pratically the same of AbcdTenorBasis::generateArguments()
	void AcdtTenorBasis::generateArguments() {
		std::vector<Real> x(4);
		x[0] = arguments_[0](0.0);//a
		 //fulfilling b with its funcitonal form. 
		x[1] = b(arguments_);//b
		x[2] = arguments_[1](0.0);//c
		x[3] = arguments_[2](0.0);//d
		//std::vector<Real> y = direct(x);
		std::vector<Real> y = x;
		if (isSimple_) {
			basis_ = shared_ptr<AbcdMathFunction>(
				new AbcdMathFunction(y[0], y[1], y[2], y[3]));
			vector<Real> c = basis_->definiteDerivativeCoefficients(0.0, tau_);
			c[0] *= tau_;
			c[1] *= tau_;
			// unaltered c[2] (the c in abcd)
			c[3] *= tau_;
			instBasis_ = shared_ptr<AbcdMathFunction>(new AbcdMathFunction(c));

		}
		else {
			instBasis_ = shared_ptr<AbcdMathFunction>(
				new AbcdMathFunction(y[0], y[1], y[2], y[3]));
			vector<Real> c =
				instBasis_->definiteIntegralCoefficients(0.0, tau_);
			c[0] /= tau_;
			c[1] /= tau_;
			// unaltered c[2] (the c in abcd)
			c[3] /= tau_;
			basis_ = shared_ptr<AbcdMathFunction>(new AbcdMathFunction(c));
		}
	}

	Real AcdtTenorBasis::b(std::vector<Parameter> arguments)
	{
		Real b;

		return b = (arguments[0](0.0)*arguments[1](0.0)) / (1 - arguments[1](0.0)*arguments[3](0.0));
	};

	std::vector<Real> AcdtTenorBasis::coeffAbcd(std::vector<Real> coeff)
	{
		std::vector<Real> coeffAbcd;
		coeffAbcd.push_back(coeff[0]);//a
		Real Implied_b = (coeff[0] * coeff[1]) / (1 - coeff[1] * coeff[3]);//b from acdt coefficients
		coeffAbcd.push_back(Implied_b);//implied b from acdt
		coeffAbcd.push_back(coeff[1]);//c
		coeffAbcd.push_back(coeff[2]);//d

		return coeffAbcd;
	};


	GlobalHelper::GlobalHelper(const boost::shared_ptr<TenorBasis>& calibratedModel,
		const std::vector<boost::shared_ptr<RateHelper>> & helpers,
		boost::shared_ptr<OptimizationMethod> & method,
		const EndCriteria & endCriteria,
		const std::vector<Real>& weights,
		const std::vector<bool>& fixParameters)
		:calibratedModel_(calibratedModel), helpers_(helpers), method_(method), endCriteria_(endCriteria),
		weights_(weights), fixParameters_(fixParameters) {};



	Real GlobalHelper::calibrationError()const
	{

		/*it asks its calibratedModel_ to calibrate itself and then asks the error*/
		calibratedModel_->calibrate(helpers_, *method_, endCriteria_, weights_, fixParameters_);//cambiare valori storati
		//Array params=calibratedModel_->params();
		Array values = calibratedModel_->problemValues();
		Real value = 0;
		for (Size i = 0; i < values.size(); ++i) {
			value += values[i]* values[i]* weights_[i];
		}
		return value;
	};

	std::vector<boost::shared_ptr<RateHelper>>& GlobalHelper::getHelpers() {
		return helpers_;
	};
	boost::shared_ptr<OptimizationMethod>& GlobalHelper::getMethod() {
		return method_;
	};
	EndCriteria& GlobalHelper::getEndCriteria() {
		return endCriteria_;
	};
	std::vector<Real>& GlobalHelper::getWeights() {
		return weights_;
	};
	std::vector<bool>& GlobalHelper::getFixParameters() {
		return fixParameters_;
	};

	//global model
	/*
	GlobalModel::GlobalModel(Size nArguments,
	const Real& coeff,
	const std::vector<boost::shared_ptr<GlobalHelper>> & helpers,
	const std::vector<int>& position,
	const int& innerErrorNumber,
	boost::shared_ptr<OptimizationMethod>& method,//not sure that this objects should be stored
	const EndCriteria& endCriteria,
	const std::vector<Real>& weights,
	const std::vector<bool>& fixParameters)
	: CalibratedModel(nArguments), helpers_(helpers), position_(position),
	innerErrorNumber_(innerErrorNumber), method_(method),
	endCriteria_(endCriteria), weights_(weights), fixParameters_(fixParameters) {
	Real y = coeff;
	arguments_[0] = ConstantParameter(y, NoConstraint());
	//generateArguments();
	}
	*/

	GlobalModel::GlobalModel(Size nArguments,
		const std::vector<Real>& coeff,
		const std::vector<boost::shared_ptr<GlobalHelper>> & helpers,
		const std::vector<int>& position
		/*boost::shared_ptr<OptimizationMethod> & method,
		const EndCriteria& endCriteria,
		const std::vector<Real>& weights,
		const std::vector<bool>& fixParameters*/)
		: CalibratedModel(nArguments), helpers_(helpers), position_(position)/*, method_(method),
		endCriteria_(endCriteria), weights_(weights), fixParameters_(fixParameters)*/ {
		std::vector<Real> y = coeff;

		for (Size i = 0; i < coeff.size(); ++i) {
			arguments_[i] = ConstantParameter(y[i], NoConstraint());
		}

		generateArguments();
	}

	/*
	void GlobalModel::generateArguments() {

		Real x;
		x = arguments_[0](0.0);

		int index = position_.size() - innerErrorNumber_;// for understanding which error is currently considered

		//set the params in the globalhelpers->calibratedModel!

		for (Size i = 0; i < helpers_.size(); ++i) {
			Array params = helpers_[i]->calibratedModel_->params(); //get parameters
			params[position_[index]] = x; // set parameters of interest
			helpers_[i]->calibratedModel_->setParams(params);// change model parameters
		}


		//If there are more than one global error-> nested calibration in nested calibration
		if (innerErrorNumber_ > 1)
		{

			//diminishing the error indicator
			innerErrorNumber_--;
			//update the index
			index = position_.size() - innerErrorNumber_;
			//Size size = 1;
			//set Argument of interest with the guess implied in calibratedModel_->params() with the coherent position index
			Array params = helpers_[0]->calibratedModel_->params(); //get parameters
			arguments_[0].setParam(0, params[position_[index]]);  // set parameters of interest
			//x = params[position_[index]]; // set parameters of interest
			//Real coeff = x;
			//Therefore, create an inner model
			//GlobalModel globalModel(size,
			//	coeff,
			//	helpers_,
			//	position_,
			//	innerErrorNumber_,
			//	method_,
			//	endCriteria_,
			//	weights_,
			//	fixParameters_);
			//globalModel.calibrate(helpers_,
			//	*method_,
			//	endCriteria_,
			//	weights_,
			//	fixParameters_);
			//	this->calibrate(helpers_,
			//		*method_,
			//		endCriteria_,
			//		weights_,
			//		fixParameters_);
	this->calibrate();
	//in order to cycle once more
	innerErrorNumber_++;
	arguments_[0].setParam(0, x);
	//index = position_.size() + innerErrorNumber_;

		}


	}; */
	void GlobalModel::generateArguments() {

		std::vector<Real> x(position_.size());

		for (Size i = 0; i < x.size(); ++i) {
			x[i] = arguments_[i](0.0);
		}


		for (Size j = 0; j < position_.size(); ++j) {
			{
				for (Size i = 0; i < helpers_.size(); ++i) {
					Array params = helpers_[i]->calibratedModel_->params(); //get parameters
					params[position_[j]] = x[j]; // set parameters of interest
					helpers_[i]->calibratedModel_->setParams(params);// change model parameters
				}
			}
		}


	};

	void GlobalModel::calibrate(
		OptimizationMethod& method,
		const EndCriteria& endCriteria,
		const std::vector<Real>& weights,
		const std::vector<bool>& fixParameters) {

		std::vector<boost::shared_ptr<CalibrationHelperBase>> cHelpers(helpers_.size());

		for (Size i = 0; i < helpers_.size(); ++i) {
			cHelpers[i] = helpers_[i];
		}
		CalibratedModel::calibrate(cHelpers, method, endCriteria,
			this->constraint(), weights, fixParameters);
	};

	/*

	void GlobalModel::calibrate(
		const std::vector<boost::shared_ptr<GlobalHelper>>& helpers,
		OptimizationMethod& method,
		const EndCriteria& endCriteria,
		const std::vector<Real>& weights,
		const std::vector<bool>& fixParameters) {

		std::vector<boost::shared_ptr<CalibrationHelperBase>> cHelpers(helpers.size());

		for (Size i = 0; i < helpers.size(); ++i) {
			cHelpers[i] = helpers[i];
		}
		CalibratedModel::calibrate(cHelpers, method, endCriteria,
			this->constraint(), weights, fixParameters);
	};

	void GlobalModel::calibrate() {

		std::vector<boost::shared_ptr<CalibrationHelperBase>> cHelpers(helpers_.size());

		for (Size i = 0; i < helpers_.size(); ++i) {
			cHelpers[i] = helpers_[i];
		}
		CalibratedModel::calibrate(cHelpers, *method_, endCriteria_,
			this->constraint(), weights_, fixParameters_);
	};
	*/

	Constraint GlobalModel::constraint() const {
		return NoConstraint();
	}

	GlobalError::GlobalError(const std::vector<boost::shared_ptr<GlobalHelper>> & helpers,
		const std::vector<int>& position,
		const int& innerErrorNumber,
		const Real& accuracy,
		const Real& min,
		const Real& max) :helpers_(helpers), position_(position), innerErrorNumber_(innerErrorNumber), accuracy_(accuracy), min_(min), max_(max) {};


	Real GlobalError::operator()(Real guess) const {

		// for understanding which error is currently considered
		int index = position_.size() - innerErrorNumber_;

		//set guess in globalhelpers->calibratedModel!
		Array params;
		for (Size i = 0; i < helpers_.size(); ++i) {
			params = helpers_[i]->calibratedModel_->params(); //get parameters
			params[position_[index]] = guess; // set parameters of interest
			helpers_[i]->calibratedModel_->setParams(params);// change model parameters
		}

		if (innerErrorNumber_ > 1) {
			innerErrorNumber_--;
			index = position_.size() - innerErrorNumber_;
			guess = params[position_[index]];
			Brent solver;
			solver.solve(*this, accuracy_, guess, min_, max_);
			innerErrorNumber_++;
		}
		else
		{
			//it asks its calibratedModel_
			for (Size i = 0; i < helpers_.size(); ++i) {
				helpers_[i]->calibratedModel_->calibrate(helpers_[i]->getHelpers(), *helpers_[i]->getMethod(), helpers_[i]->getEndCriteria(), helpers_[i]->getWeights(), helpers_[i]->getFixParameters());//cambiare valori storati
			}
			this->value();//set the error
		}

		return value_;
	}

	void GlobalError::value(void) const{
		Real value_ = 0;
		Array values;
		for (Size j = 0; j < helpers_.size(); ++j) {
			values = helpers_[j]->calibratedModel_->problemValues();
			for (Size i = 0; i < values.size(); ++i) {
				value_ += values[i] * values[i];
			}
		}
		
	};
}




