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

#if defined(HAVE_CONFIG_H)     // Dynamically created by configure
    #include <qlo/config.hpp>
#endif
#include <qlo/tenorbasis.hpp>

#include <ql/math/abcdmathfunction.hpp>
#include <ql/math/polynomialmathfunction.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/experimental/tenorbasis/tenorbasis.hpp>
#include <ql/experimental/tenorbasis/tenorbasiscalibration.hpp>

#include <oh/repository.hpp>

using boost::shared_ptr;
using ObjectHandler::LibraryObject;

namespace QuantLibAddin {
   
	
    TenorBasis::TenorBasis(
                       const shared_ptr<ObjectHandler::ValueObject>& p,
                       bool permanent)
    : CalibratedModel(p, permanent){}

    AbcdTenorBasis::AbcdTenorBasis(
        const shared_ptr<ObjectHandler::ValueObject>& p,
        shared_ptr<QuantLib::IborIndex> iborIndex,
        boost::shared_ptr<QuantLib::IborIndex> baseIborIndex,
        QuantLib::Date referenceDate,
        bool isSimple,
        const std::vector<QuantLib::Real>& coeff,
        bool permanent)
    : TenorBasis(p, permanent) {
        libraryObject_ = shared_ptr<QuantLib::AbcdTenorBasis>(new
            QuantLib::AbcdTenorBasis(iborIndex, baseIborIndex, referenceDate,
                                     isSimple,coeff));
    }

    AcdtTenorBasis::AcdtTenorBasis(
        const shared_ptr<ObjectHandler::ValueObject>& p,
        shared_ptr<QuantLib::IborIndex> iborIndex,
        boost::shared_ptr<QuantLib::IborIndex> baseIborIndex,
        QuantLib::Date referenceDate,
        bool isSimple,
        const std::vector<QuantLib::Real>& coeff,
        bool permanent)
    : AbcdTenorBasis(p, iborIndex, baseIborIndex, referenceDate, isSimple,
                     coeff, permanent) {
        libraryObject_ = shared_ptr<QuantLib::AcdtTenorBasis>(new
            QuantLib::AcdtTenorBasis(iborIndex, baseIborIndex, referenceDate,
                isSimple, coeff));
    }



	AcdtTenorBasis::AcdtTenorBasis(
		const shared_ptr<ObjectHandler::ValueObject>& p,
		shared_ptr<QuantLib::IborIndex> iborIndex,
		boost::shared_ptr<QuantLib::IborIndex> baseIborIndex,
		QuantLib::Date referenceDate,
		bool isSimple,
		const std::vector<QuantLib::Real>& coeff,
		bool rebuilding,
		bool permanent)
		: AbcdTenorBasis(p, iborIndex, baseIborIndex, referenceDate, isSimple,
			coeff, permanent) {
		libraryObject_ = shared_ptr<QuantLib::AcdtTenorBasis>(new
			QuantLib::AcdtTenorBasis(iborIndex, baseIborIndex, referenceDate,
				isSimple, coeff,rebuilding));
	}

	
	GlobalHelper::GlobalHelper(
		const shared_ptr<ObjectHandler::ValueObject>& p,
		const boost::shared_ptr<QuantLib::TenorBasis>& calibratedModel,
		const std::vector<boost::shared_ptr<QuantLib::RateHelper>> & helpers,
		boost::shared_ptr<QuantLib::OptimizationMethod> & method,
		const QuantLib::EndCriteria & endCriteria,
		const std::vector<QuantLib::Real>& weights,
		const std::vector<bool>& fixParameters,
		bool permanent)
		: CalibrationHelperBase(p, permanent) {
		libraryObject_ = shared_ptr<QuantLib::GlobalHelper>(new
			QuantLib::GlobalHelper(calibratedModel, helpers, method,
				endCriteria, weights, fixParameters));
	}


	GlobalModel::GlobalModel(
		const shared_ptr<ObjectHandler::ValueObject>& p,
		QuantLib::Size nArguments,
		const std::vector<QuantLib::Real>& coeff,
		const std::vector<boost::shared_ptr<QuantLib::GlobalHelper>> & helpers,
		const std::vector<QuantLib::Integer>& position,
		bool permanent)
		: CalibratedModel(p, permanent) {
		libraryObject_ = shared_ptr<QuantLib::GlobalModel>(new
			QuantLib::GlobalModel(nArguments, coeff, helpers,
				position));
	}


    PolynomialTenorBasis::PolynomialTenorBasis(
        const shared_ptr<ObjectHandler::ValueObject>& p,
        shared_ptr<QuantLib::IborIndex> iborIndex,
        boost::shared_ptr<QuantLib::IborIndex> baseIborIndex,
        QuantLib::Date referenceDate,
        bool isSimple,
        const std::vector<QuantLib::Real>& coeff,
        bool permanent)
    : TenorBasis(p, permanent) {
        libraryObject_ = shared_ptr<QuantLib::PolynomialTenorBasis>(new
            QuantLib::PolynomialTenorBasis(iborIndex, baseIborIndex, referenceDate,
                                           isSimple, coeff));
    }

    AbcdCalibration2::AbcdCalibration2(
        const shared_ptr<ObjectHandler::ValueObject>& properties,
        const std::vector<QuantLib::Time>& t,
        const std::vector<QuantLib::Rate>& r,
        const std::vector<QuantLib::Real>& w,
        std::vector<QuantLib::Real> coeff,
        const std::vector<bool>& fixedCoeff,
        const shared_ptr<QuantLib::EndCriteria> endCriteria,
        const shared_ptr<QuantLib::OptimizationMethod> method,
        bool permanent)
    : LibraryObject<QuantLib::AbcdCalibration2>(properties, permanent) {

        libraryObject_ = shared_ptr<QuantLib::AbcdCalibration2>(new
            QuantLib::AbcdCalibration2(t, r, w, coeff, fixedCoeff,
            endCriteria, method));
    }

    PolynomialCalibration::PolynomialCalibration(
        const boost::shared_ptr<ObjectHandler::ValueObject>& properties,
        const std::vector<QuantLib::Time>& t,
        const std::vector<QuantLib::Rate>& r,
        const std::vector<QuantLib::Real>& w,
        std::vector<QuantLib::Real> coeff,
        const std::vector<bool>& fixedCoeff,
        const boost::shared_ptr<QuantLib::EndCriteria> endCriteria,
        const boost::shared_ptr<QuantLib::OptimizationMethod> method,
        bool permanent)
    : LibraryObject<QuantLib::PolynomialCalibration>(properties, permanent) {

        libraryObject_ = shared_ptr<QuantLib::PolynomialCalibration>(new
            QuantLib::PolynomialCalibration(t, r, w, coeff, fixedCoeff,
            endCriteria, method));
    }

    TenorBasisYieldTermStructure::TenorBasisYieldTermStructure(
            const shared_ptr<ObjectHandler::ValueObject>& properties,
            const shared_ptr<QuantLib::TenorBasis>& basis,
            bool permanent)
    :YieldTermStructure(properties, permanent){
        libraryObject_ = shared_ptr<QuantLib::TenorBasisYieldTermStructure>(new
            QuantLib::TenorBasisYieldTermStructure(basis));
    }

    DiscountCorrectedTermStructure::DiscountCorrectedTermStructure(
            const shared_ptr<ObjectHandler::ValueObject>& properties,
            const QuantLib::Handle<QuantLib::YieldTermStructure>& baseCurve,
            const std::vector<boost::shared_ptr<QuantLib::RateHelper> >& instruments,
            QuantLib::Real accuracy,
            bool permanent)
    : YieldTermStructure(properties, permanent){
            libraryObject_ = shared_ptr<QuantLib::DiscountCorrectedTermStructure>(new
                QuantLib::DiscountCorrectedTermStructure(baseCurve, instruments, accuracy));
    }

    TenorBasisForwardRateCurve::TenorBasisForwardRateCurve(
        const shared_ptr<ObjectHandler::ValueObject>& properties,
        const shared_ptr<QuantLib::TenorBasis>& basis,
        bool permanent)
    : ForwardRateCurve(properties, permanent){
        libraryObject_ = shared_ptr<QuantLib::TenorBasisForwardRateCurve>(new
            QuantLib::TenorBasisForwardRateCurve(basis));
    }

    ForwardCorrectedTermStructure::ForwardCorrectedTermStructure(
        const shared_ptr<ObjectHandler::ValueObject>& properties,
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
        bool permanent)
    : ForwardRateCurve(properties, permanent){
        libraryObject_ = shared_ptr<QuantLib::ForwardCorrectedTermStructure>(new
            QuantLib::ForwardCorrectedTermStructure(fwdFamilyName, 
                                                    fwdTenor, 
                                                    fwdSettlementDays, 
                                                    fwdCurrency,
                                                    fwdFixingCalendar, 
                                                    fwdConvention, 
                                                    fwdEndOfMonth,
                                                    fwdDayCounter,
                                                    baseCurve, 
                                                    instruments, 
                                                    accuracy));
    }

}