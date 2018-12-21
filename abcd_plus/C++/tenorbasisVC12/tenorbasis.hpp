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

#ifndef quantlib_tenor_basis_hpp
#define quantlib_tenor_basis_hpp

#include <ql/math/abcdmathfunction.hpp>
#include <ql/math/polynomialmathfunction.hpp>
#include <ql/handle.hpp>
#include <ql/models/model.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/termstructures/interpolatedcurve.hpp>
#include <ql/termstructures/iterativebootstrap.hpp>
#include <ql/termstructures/yield/bootstraptraits.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/experimental/tenorbasis/forwardhelpers.hpp>
#include <ql/experimental/tenorbasis/fwdratecurve.hpp>


namespace QuantLib {

	class IborIndex;

	//! Tenor (simple) basis between a given forwarding curve and a base curve
	/*!  */
	class TenorBasis : public CalibratedModel {
	public:
		TenorBasis(Size nArguments,
			boost::shared_ptr<IborIndex> iborIndex,
			boost::shared_ptr<IborIndex> baseIborIndex,
			Date referenceDate = Date());
		//! \name Interface
		//@{
		//! tenor (simple) basis as function of Date
		Spread value(Date d) const;
		//! tenor (simple) basis as function of Time
		Real value(const Array& params,
			const std::vector<boost::shared_ptr<RateHelper>>&h);
		//! tenor (simple) basis as function of Time
		virtual Spread value(Time t) const = 0;
		//! tenor (continuous) basis as function of Date
		Spread instBasisValue(Date d) const;
		//! tenor (continuous) basis as function of Time
		virtual Spread instBasisValue(Time t) const = 0;

		//! fixed tenor forward rate obtained from simple basis
		Rate tenorForwardRate(Date d) const;
		//! fixed tenor forward rate obtained from simple basis
		Rate tenorForwardRate(Time t) const;

		// fixed tenor forward rate obtained as integral of inst. cont. basis
		Rate forwardRate(Date d) const;
		// forward rate obtained as integral of instantaneous continuous basis
		Rate forwardRate(Date d1,
			Date d2) const;
		// forward rate obtained as integral of instantaneous continuous basis
		Rate forwardRate(Time t1,
			Time t2) const;

		// accrual factor obtained as integral of inst. cont. basis
		Rate accrualFactor(Date d1,
			Date d2) const;
		// accrual factor obtained as integral of inst. cont. basis
		Rate accrualFactor(Time t1,
			Time t2) const;

		//! simple basis parametrization coefficients
		virtual const std::vector<Real>& coefficients() const = 0;
		//! instantaneous continuous basis parametrization coefficients
		virtual const std::vector<Real>& instCoefficients() const = 0;
		//@}

		//! \name Dates and Time
		//@{
		//! the day counter used for date/time conversion
		DayCounter dayCounter() const { return dateTimeDC_; }
		//! date-to-time conversion
		Time timeFromReference(Date d) const;
		//! time-to-date conversion
		Date dateFromTime(Time t) const;
		//@}

		//! Inspectors
		//@{
		//! settlement date for which t=0
		const Date& referenceDate() const { return referenceDate_; }
		//! IborIndex proving the forwarding curve
		const boost::shared_ptr<IborIndex>& iborIndex() const;
		//! Base IborIndex whose forwarding curve is used as basis
		const boost::shared_ptr<IborIndex>& baseIborIndex() const;
		//@}

		// calibration on pseudo-discount factor
		void calibrate(
			const std::vector<boost::shared_ptr<RateHelper> >&,
			OptimizationMethod& method,
			const EndCriteria& endCriteria
			= EndCriteria(1000, 100, 1.0e-8, 0.3e-4, 0.3e-4),
			const std::vector<Real>& weights = std::vector<Real>(),
			const std::vector<bool>& fixParameters = std::vector<bool>());

		// calibration on forward rate
		void forwardCalibrate(
			const std::vector<boost::shared_ptr<ForwardHelper> >&,
			OptimizationMethod& method,
			const EndCriteria& endCriteria
			= EndCriteria(1000, 100, 1.0e-8, 0.3e-4, 0.3e-4),
			const std::vector<Real>& weights = std::vector<Real>(),
			const std::vector<bool>& fixParameters = std::vector<bool>());

	protected:
		//! \name Integral functions
		//@{

		/*! \f[ I(d) = \int_{d}^{d+\tau} b(s)ds \f]
			with \f[ b(t) \f] being the instantaneous continuous basis
			 and \f[ \tau \f] being the iborIndex tenor */
		Real integrate_(Date d) const;

		/*! \f[ I(d1, d2) = \int_{d1}^{d2} b(s)ds \f]
			with \f[ b(t) \f] being the instantaneous continuous basis */
		Real integrate_(Date d1,
			Date d2) const;

		/*! \f[ I(t1, t2) = \int_{t1}^{t2} b(s)ds \f]
			with \f[ b(t) \f] being the instantaneous continuous basis
			TODO: possibly implement numerical integration as default */
		virtual Real integrate_(Time t1,
			Time t2) const = 0;
		//@}
		virtual Constraint constraint() const;
		boost::shared_ptr<IborIndex> iborIndex_, baseIborIndex_;
		Date referenceDate_;

		DayCounter dateTimeDC_, dc_;
		BusinessDayConvention bdc_;
		bool eom_;
		Calendar cal_;
		Period tenor_;
		Time tau_;
		Real sign_;
	};

	class AbcdTenorBasis : public TenorBasis {
	public:
		AbcdTenorBasis(boost::shared_ptr<IborIndex> iborIndex,
			boost::shared_ptr<IborIndex> baseIborIndex,
			Date referenceDate,
			bool isSimple,
			const std::vector<Real>& coeff);
		//! \name TenorBasis Interface
		//@{
		Spread value(Time t)const;
		Spread instBasisValue(Time t) const { return (*instBasis_)(t); };
		const std::vector<Real>& coefficients() const;
		const std::vector<Real>& instCoefficients() const;
		//@}

		//! date at which the simple tenor basis reaches maximum, if any
		Date maximumLocation() const;
		//! maximum values for the simple tenor basis, if any
		Spread maximumValue() const;
		//! long term simple tenor basis
		Spread longTermValue() const { return basis_->d(); }
		//! date at which the continuous tenor basis reaches maximum, if any
		Date instBasisMaximumLocation() const;
		//! maximum values for the continuous tenor basis, if any
		Spread instBasisMaximumValue() const;
		//! long term continuous tenor basis
		Spread instBasisLongTermValue() const { return instBasis_->d(); }
	protected:
		//! \name TenorBasis Interface
		//@{
		Real integrate_(Time t1,
			Time t2) const;
		//@}
		//! \name CalibratedModel Interface
		//@{
		void generateArguments();
		//@}
		virtual Constraint constraint() const;
		boost::shared_ptr<AbcdMathFunction> basis_, instBasis_;
		bool isSimple_;
	};

	class PolynomialTenorBasis : public TenorBasis {
	public:
		PolynomialTenorBasis(boost::shared_ptr<IborIndex> iborIndex,
			boost::shared_ptr<IborIndex> baseIborIndex,
			Date referenceDate,
			bool isSimple,
			const std::vector<Real>& coeff);
		//! \name TenorBasis Interface
		//@{
		Spread value(Time t) const { return (*basis_)(t); }
		Spread instBasisValue(Time t) const { return (*instBasis_)(t); }
		const std::vector<Real>& coefficients() const;
		const std::vector<Real>& instCoefficients() const;
		//@}
	protected:
		//! \name TenorBasis Interface
		//@{
		Real integrate_(Time t1,
			Time t2) const;
		//@}
		//! \name CalibratedModel Interface
		//@{
		void generateArguments();
		//@}
		boost::shared_ptr<PolynomialFunction> basis_, instBasis_;
		bool isSimple_;
	};

	//! Helper class for the calibration through the pseudo-discount factors
	class TenorBasisYieldTermStructure : public YieldTermStructure {
	public:
		TenorBasisYieldTermStructure(const boost::shared_ptr<TenorBasis>& basis);
		const Date& referenceDate() const;
		Calendar calendar() const;
		Natural settlementDays() const;
		Date maxDate() const;
	private:
		DiscountFactor discountImpl(Time) const;
		boost::shared_ptr<TenorBasis> basis_;
	};

	//! Helper class for the calibration directly on forward rates
	class TenorBasisForwardRateCurve : public ForwardRateCurve {
	public:
		TenorBasisForwardRateCurve(const boost::shared_ptr<TenorBasis>& basis);
		const Date& referenceDate() const;
		Calendar calendar() const;
		Natural settlementDays() const;
		Date maxDate() const;
	private:
		Rate forwardRate(Time t, bool extrapolate = false) const;
		boost::shared_ptr<TenorBasis> basis_;
	};

	//! continuous basis k bootstrapping
	class DiscountCorrectedTermStructure : public YieldTermStructure,
		protected InterpolatedCurve<Linear>,
		public LazyObject {
	public:
		typedef Discount traits_type;
		typedef Linear interpolator_type;
		DiscountCorrectedTermStructure(
			const Handle<YieldTermStructure>& bestFitCurve,
			const std::vector<boost::shared_ptr<RateHelper> >& instruments,
			Real accuracy = 1.0e-12);
		const Date& referenceDate() const;
		DayCounter dayCounter() const;
		Calendar calendar() const;
		Natural settlementDays() const;
		Date maxDate() const;
		const std::vector<Time>& times() const;
		const std::vector<Date>& dates() const;
		const std::vector<Real>& data() const;
		void update();
	private:
		DiscountFactor discountImpl(Time) const;
		void performCalculations() const;
		// data members
		Handle<YieldTermStructure> bestFitCurve_;
		std::vector<boost::shared_ptr<RateHelper> > instruments_;
		Real accuracy_;
		mutable std::vector<Date> dates_;

		// bootstrapper classes are declared as friend to manipulate
		// the curve data. They might be passed the data instead, but
		// it would increase the complexity---which is high enough
		// already.
		friend class IterativeBootstrap<DiscountCorrectedTermStructure>;
		friend class BootstrapError<DiscountCorrectedTermStructure>;
		IterativeBootstrap<DiscountCorrectedTermStructure> bootstrap_;
	};

	//! continuous basis k bootstrapping
	class ForwardCorrectedTermStructure : public ForwardRateCurve,
		protected InterpolatedCurve<Linear>,
		public LazyObject {
	public:
		typedef CorrectionFactorTraits traits_type;
		typedef Linear interpolator_type;
		ForwardCorrectedTermStructure(
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
			Real accuracy = 1.0e-12);
		const Date& referenceDate() const;
		DayCounter dayCounter() const;
		Calendar calendar() const;
		Natural settlementDays() const;
		Date maxDate() const;
		const std::vector<Time>& times() const;
		const std::vector<Date>& dates() const;
		const std::vector<Real>& data() const;
		void update();
	private:
		Rate forwardRate(Time t, bool extrapolate = false) const;
		void performCalculations() const;
		// data members
		Handle<ForwardRateCurve> bestFitCurve_;
		std::vector<boost::shared_ptr<ForwardHelper> > instruments_;
		Real accuracy_;
		mutable std::vector<Date> dates_;

		// bootstrapper classes are declared as friend to manipulate
		// the curve data. They might be passed the data instead, but
		// it would increase the complexity---which is high enough
		// already.
		friend class IterativeBootstrap<ForwardCorrectedTermStructure>;
		friend class BootstrapError<ForwardCorrectedTermStructure>;
		IterativeBootstrap<ForwardCorrectedTermStructure> bootstrap_;
	};

	class AcdtTenorBasis : public AbcdTenorBasis {
	public:
		AcdtTenorBasis(boost::shared_ptr<IborIndex> iborIndex,
			boost::shared_ptr<IborIndex> baseIborIndex,
			Date referenceDate,
			bool isSimple,
			const std::vector<Real>& coeff);

		void generateArguments();
		Real b(std::vector<Parameter> param);
		std::vector<Real> coeffAbcd(std::vector<Real> coeff);
	};

	class GlobalHelper : public CalibrationHelperBase {

	public:
		GlobalHelper(const boost::shared_ptr<TenorBasis>& calibratedModel,
			const std::vector<boost::shared_ptr<RateHelper>> & helpers,
			boost::shared_ptr<OptimizationMethod> & method,
			const EndCriteria  & endCriteria,
			const std::vector<Real> & weights,
			const std::vector<bool> & fixParameters);

		Real calibrationError()const;
		std::vector<boost::shared_ptr<RateHelper>>& getHelpers();
		boost::shared_ptr<OptimizationMethod>& getMethod();
		EndCriteria& getEndCriteria();
		std::vector<Real>& getWeights();
		std::vector<bool>& getFixParameters();

		boost::shared_ptr<TenorBasis> calibratedModel_;
	protected:

		std::vector<boost::shared_ptr<RateHelper>> helpers_;
		boost::shared_ptr<OptimizationMethod> method_;
		EndCriteria endCriteria_;
		std::vector<Real> weights_;
		std::vector<bool> fixParameters_;

	};

	class GlobalModel : public CalibratedModel {

	public:
		GlobalModel(Size nArguments,
			const std::vector<Real>& coeff,
			const std::vector<boost::shared_ptr<GlobalHelper>> & helpers,
			const std::vector<int>& position
			/*boost::shared_ptr<OptimizationMethod> & method,
			const EndCriteria& endCriteria,
			const std::vector<Real>& weights,
			const std::vector<bool>& fixParameters*/);

		void generateArguments();
		virtual Constraint constraint() const;

		void calibrate(
			OptimizationMethod& method,
			const EndCriteria& endCriteria,
			const std::vector<Real>& weights,
			const std::vector<bool>& fixParameters);
		/*
		void calibrate(
			const std::vector<boost::shared_ptr<GlobalHelper>>& helpers,
			OptimizationMethod& method,
			const EndCriteria& endCriteria,
			const std::vector<Real>& weights,
			const std::vector<bool>& fixParameters);

		void calibrate();
		*/
	protected:

		std::vector<int> position_;
		std::vector<boost::shared_ptr<GlobalHelper>> helpers_;
		/*boost::shared_ptr<OptimizationMethod> method_;
		EndCriteria endCriteria_;
		std::vector<Real> weights_;
		std::vector<bool> fixParameters_;*/

	};

/*
	class GlobalModel : public CalibratedModel {

	public:
		GlobalModel(Size nArguments,
			const Real& coeff,
			const std::vector<boost::shared_ptr<GlobalHelper>> & helpers,
			const std::vector<int>& position,
			const int& innerErrorNumber,
			boost::shared_ptr<OptimizationMethod> & method,
			const EndCriteria& endCriteria,
			const std::vector<Real>& weights,
			const std::vector<bool>& fixParameters);

		void generateArguments();
		virtual Constraint constraint() const;
		//real calibration
		void calibrate(
			const std::vector<boost::shared_ptr<GlobalHelper>>& helpers,
			OptimizationMethod& method,
			const EndCriteria& endCriteria,
			const std::vector<Real>& weights,
			const std::vector<bool>& fixParameters);
		void calibrate();

	protected:

		std::vector<int> position_;
		int innerErrorNumber_;
		std::vector<boost::shared_ptr<GlobalHelper>> helpers_;
		boost::shared_ptr<OptimizationMethod> method_;
		EndCriteria endCriteria_;
		std::vector<Real> weights_;
		std::vector<bool> fixParameters_;

	};
*/

	class GlobalError {
	public:
		GlobalError(const std::vector<boost::shared_ptr<GlobalHelper>> & helpers,
			const std::vector<int>& position,
			const int& innerErrorNumber,
			const Real& accuracy,
			const Real& min,
			const Real& max);

		Real operator()(Real guess)const;
		void value(void)const;
	private:
		std::vector<boost::shared_ptr<GlobalHelper>> helpers_;
		Brent firstSolver_;
		mutable int innerErrorNumber_;
		std::vector<int> position_;
		Real accuracy_;
		Real min_;
		Real max_;
		mutable Real value_;
	};
}

#endif
