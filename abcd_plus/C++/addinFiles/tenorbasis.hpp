/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Ferdinando Ametrano
 Copyright (C) 2015 Paolo Mazzocchi
 Copyright (C) 2019 Gabriele Giudici

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

#ifndef qla_tenorbasis_hpp
#define qla_tenorbasis_hpp

#include <oh/libraryobject.hpp>
#include <ql/types.hpp>

#include <qlo/calibrationhelper.hpp>
#include <qlo/model.hpp>
#include <qlo/yieldtermstructures.hpp>

namespace QuantLib {
    class TenorBasis;
    class Date;
    class Quote;
    class AbcdMathFunction;
    class PolynomialFunction;
    class IborIndex;
    class CalibratedModel;
    class YieldTermStructure;
	class GlobalHelper;
    // for calibration
	class CalibrationHelperBase;
    class PolynomialCalibration;
    class AbcdCalibration2;
    class EndCriteria;
    class OptimizationMethod;

    template <class T>
    class Handle;
}

namespace QuantLibAddin {


    class TenorBasis : public CalibratedModel {
      public:
        TenorBasis(const boost::shared_ptr<ObjectHandler::ValueObject>& p,
                   bool permanent);
    };

    class AbcdTenorBasis : public TenorBasis {
    public:
        AbcdTenorBasis(
            const boost::shared_ptr<ObjectHandler::ValueObject>& p,
            boost::shared_ptr<QuantLib::IborIndex> iborIndex,
            boost::shared_ptr<QuantLib::IborIndex> baseIborIndex,
            QuantLib::Date referenceDate,
            bool isSimple,
            const std::vector<QuantLib::Real>& coeff,
            bool permanent);
    };

    class AcdtTenorBasis : public AbcdTenorBasis {
    public:
        AcdtTenorBasis(
            const boost::shared_ptr<ObjectHandler::ValueObject>& p,
            boost::shared_ptr<QuantLib::IborIndex> iborIndex,
            boost::shared_ptr<QuantLib::IborIndex> baseIborIndex,
            QuantLib::Date referenceDate,
            bool isSimple,
            const std::vector<QuantLib::Real>& coeff,
            bool permanent);
 
		AcdtTenorBasis(
			const boost::shared_ptr<ObjectHandler::ValueObject>& p,
			boost::shared_ptr<QuantLib::IborIndex> iborIndex,
			boost::shared_ptr<QuantLib::IborIndex> baseIborIndex,
			QuantLib::Date referenceDate,
			bool isSimple,
			const std::vector<QuantLib::Real>& coeff,
			bool rebuilding,
			bool permanent);

	};

	class GlobalHelper : public CalibrationHelperBase {
	public:
		GlobalHelper(
			const boost::shared_ptr<ObjectHandler::ValueObject>& p, 
			const boost::shared_ptr<QuantLib::TenorBasis>& calibratedModel,
			const std::vector<boost::shared_ptr<QuantLib::RateHelper>> & helpers,
			boost::shared_ptr<QuantLib::OptimizationMethod> & method,
			const QuantLib::EndCriteria  & endCriteria,
			const std::vector<QuantLib::Real> & weights,
			const std::vector<bool> & fixParameters,
			bool permanent);
	};

	class GlobalModel : public CalibratedModel {
	public:
		GlobalModel(
			const boost::shared_ptr<ObjectHandler::ValueObject>& p,
			QuantLib::Size nArguments,
			const std::vector<QuantLib::Real>& coeff,
			const std::vector<boost::shared_ptr<QuantLib::GlobalHelper>> & helpers,
			const std::vector<QuantLib::Integer>& position,
			bool permanent);
	};

    class PolynomialTenorBasis : public TenorBasis {
    public:
        PolynomialTenorBasis(
            const boost::shared_ptr<ObjectHandler::ValueObject>& p,
            boost::shared_ptr<QuantLib::IborIndex> iborIndex,
            boost::shared_ptr<QuantLib::IborIndex> baseIborIndex,
            QuantLib::Date referenceDate,
            bool isSimple,
            const std::vector<QuantLib::Real>& coeff,
            bool permanent);
    };

    class AbcdCalibration2 :
        public ObjectHandler::LibraryObject<QuantLib::AbcdCalibration2> {
    public:
        AbcdCalibration2(
            const boost::shared_ptr<ObjectHandler::ValueObject>& properties,
            const std::vector<QuantLib::Time>& t,
            const std::vector<QuantLib::Rate>& r,
            const std::vector<QuantLib::Real>& w,
            std::vector<QuantLib::Real> coeff,
            const std::vector<bool>& fixedCoeff,
            const boost::shared_ptr<QuantLib::EndCriteria> endCriteria,
            const boost::shared_ptr<QuantLib::OptimizationMethod> method,
            bool permanent);
    };

    class PolynomialCalibration :
        public ObjectHandler::LibraryObject<QuantLib::PolynomialCalibration> {
    public:
        PolynomialCalibration(
            const boost::shared_ptr<ObjectHandler::ValueObject>& properties,
            const std::vector<QuantLib::Time>& t,
            const std::vector<QuantLib::Rate>& rates,
            const std::vector<QuantLib::Real>& weights,
            std::vector<QuantLib::Real> coeff,
            const std::vector<bool>& fixedCoeff,
            const boost::shared_ptr<QuantLib::EndCriteria> endCriteria,
            const boost::shared_ptr<QuantLib::OptimizationMethod> method,
            bool permanent);
    };

    class TenorBasisYieldTermStructure : public YieldTermStructure {
      public:
          TenorBasisYieldTermStructure(
            const boost::shared_ptr<ObjectHandler::ValueObject>& properties,    
            const boost::shared_ptr<QuantLib::TenorBasis>& basis,
            bool permanent);
    };

    class DiscountCorrectedTermStructure : public YieldTermStructure {
    public:
        DiscountCorrectedTermStructure(
            const boost::shared_ptr<ObjectHandler::ValueObject>& properties,
            const QuantLib::Handle<QuantLib::YieldTermStructure>& baseCurve,
            const std::vector<boost::shared_ptr<QuantLib::RateHelper> >& instruments,
            QuantLib::Real accuracy,
            bool permanent);
    };

    class TenorBasisForwardRateCurve : public ForwardRateCurve {
    public:
        TenorBasisForwardRateCurve(
            const boost::shared_ptr<ObjectHandler::ValueObject>& properties,
            const boost::shared_ptr<QuantLib::TenorBasis>& basis,
            bool permanent);
    };

    class ForwardCorrectedTermStructure : public ForwardRateCurve {
    public:
        ForwardCorrectedTermStructure(
            const boost::shared_ptr<ObjectHandler::ValueObject>& properties,
            const std::string& fwdFamilyName,
            const QuantLib::Period& fwdTenor,
            QuantLib::Natural fwdSettlementDays,
            const QuantLib::Currency& fwdCurrency,
            const QuantLib::Calendar& fwdFixingCalendar,
            QuantLib::BusinessDayConvention fwdConvention,
            bool fwdEndOfMonth,
            const QuantLib::DayCounter& fwdDayCounter,
            const QuantLib::Handle<QuantLib::ForwardRateCurve>& baseCurve,
            const std::vector<boost::shared_ptr<QuantLib::ForwardHelper> >& instruments,
            QuantLib::Real accuracy,
            bool permanent);
    };

}

#endif
