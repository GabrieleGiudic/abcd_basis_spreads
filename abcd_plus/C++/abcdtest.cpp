
/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */



/*!

Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl

Copyright (C) 2004 Ferdinando Ametrano

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



/*  This example shows how to set up a Term Structure and then price a simple

swap.

*/



// the only header you need to use QuantLib

#include <ql/quantlib.hpp>



#ifdef BOOST_MSVC

	/* Uncomment the following lines to unmask floating-point

	exceptions. Warning: unpredictable results can arise...



	See http://www.wilmott.com/messageview.cfm?catid=10&threadid=9481

	Is there anyone with a definitive word about this?

	*/

	// #include <float.h>

	// namespace { unsigned int u = _controlfp(_EM_INEXACT, _MCW_EM); }

#endif



#include <boost/timer.hpp>

#include <iostream>

#include <iomanip>



using namespace QuantLib;

Real error(const Array& values)
{
	Real value = 0;
	for (Size i = 0; i < values.size(); ++i) {
		value += values[i] * values[i];
	}
	return value;
}

Real t(const std::vector<Real> & abcd) {
	Real t;
	return t = 1 / abcd[2] - abcd[0] / abcd[1];
}


Real b(const std::vector<Real> & acdt) {
	Real b;
	return b = (acdt[0] * acdt[1]) / (1 - acdt[3] * acdt[1]);
}



#ifdef BOOST_MSVC

#  ifdef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN

#    include <ql/auto_link.hpp>

#    define BOOST_LIB_NAME boost_system

#    include <boost/config/auto_link.hpp>

#    undef BOOST_LIB_NAME

#    define BOOST_LIB_NAME boost_thread

#    include <boost/config/auto_link.hpp>

#    undef BOOST_LIB_NAME

#  endif

#endif



#if defined(QL_ENABLE_SESSIONS)

namespace QuantLib {



	Integer sessionId() { return 0; }



}

#endif





int main(int, char*[]) {




	boost::timer timer;

	std::cout << std::endl;



	/*********************

	***  MARKET DATA  ***

	*********************/



	Calendar calendar = TARGET();

	Date settlementDate(22, September, 2004);

	// must be a business day

	settlementDate = calendar.adjust(settlementDate);



	Integer fixingDays = 2;

	Date todaysDate = calendar.advance(settlementDate, -fixingDays, Days);
	// nothing to do with Date::todaysDate

	Settings::instance().evaluationDate() = todaysDate;





	todaysDate = Settings::instance().evaluationDate();

	std::cout << "Today: " << todaysDate.weekday()

		<< ", " << todaysDate << std::endl;



	std::cout << "Settlement date: " << settlementDate.weekday()

		<< ", " << settlementDate << std::endl;



	// deposits

	Rate d1wQuote = 0.0382;

	Rate d1mQuote = 0.0372;

	Rate d3mQuote = 0.0363;

	Rate d6mQuote = 0.0353;

	Rate d9mQuote = 0.0348;

	Rate d1yQuote = 0.0345;

	// FRAs

	Rate fra3x6Quote = 0.037125;

	Rate fra6x9Quote = 0.037125;

	Rate fra6x12Quote = 0.037125;

	// futures

	Real fut1Quote = 96.2875;

	Real fut2Quote = 96.7875;

	Real fut3Quote = 96.9875;

	Real fut4Quote = 96.6875;

	Real fut5Quote = 96.4875;

	Real fut6Quote = 96.3875;

	Real fut7Quote = 96.2875;

	Real fut8Quote = 96.0875;

	// swaps

	Rate s2yQuote = 0.037125;

	Rate s3yQuote = 0.0398;

	Rate s5yQuote = 0.0443;

	Rate s10yQuote = 0.05165;

	Rate s15yQuote = 0.055175;

	// OIS

	Rate ois1wQuote = 0.0382;

	Rate ois1mQuote = 0.0372;

	Rate ois3mQuote = 0.0363;

	Rate ois6mQuote = 0.0353;

	Rate ois9mQuote = 0.0348;

	Rate ois1yQuote = 0.0345;

	Rate ois2yQuote = 0.037125;

	Rate ois3yQuote = 0.0398;

	Rate ois5yQuote = 0.0443;

	Rate ois10yQuote = 0.05165;

	Rate ois15yQuote = 0.055175;





	/********************

	***    QUOTES    ***

	********************/



	// SimpleQuote stores a value which can be manually changed;

	// other Quote subclasses could read the value from a database

	// or some kind of data feed.



	// deposits

	boost::shared_ptr<Quote> d1wRate(new SimpleQuote(d1wQuote));

	boost::shared_ptr<Quote> d1mRate(new SimpleQuote(d1mQuote));

	boost::shared_ptr<Quote> d3mRate(new SimpleQuote(d3mQuote));

	boost::shared_ptr<Quote> d6mRate(new SimpleQuote(d6mQuote));

	boost::shared_ptr<Quote> d9mRate(new SimpleQuote(d9mQuote));

	boost::shared_ptr<Quote> d1yRate(new SimpleQuote(d1yQuote));

	// FRAs

	boost::shared_ptr<Quote> fra3x6Rate(new SimpleQuote(fra3x6Quote));

	boost::shared_ptr<Quote> fra6x9Rate(new SimpleQuote(fra6x9Quote));

	boost::shared_ptr<Quote> fra9x12Rate(new SimpleQuote(fra6x12Quote));

	// futures

	boost::shared_ptr<Quote> fut1Price(new SimpleQuote(fut1Quote));

	boost::shared_ptr<Quote> fut2Price(new SimpleQuote(fut2Quote));

	boost::shared_ptr<Quote> fut3Price(new SimpleQuote(fut3Quote));

	boost::shared_ptr<Quote> fut4Price(new SimpleQuote(fut4Quote));

	boost::shared_ptr<Quote> fut5Price(new SimpleQuote(fut5Quote));

	boost::shared_ptr<Quote> fut6Price(new SimpleQuote(fut6Quote));

	boost::shared_ptr<Quote> fut7Price(new SimpleQuote(fut7Quote));

	boost::shared_ptr<Quote> fut8Price(new SimpleQuote(fut8Quote));

	// swaps

	boost::shared_ptr<Quote> s2yRate(new SimpleQuote(s2yQuote));

	boost::shared_ptr<Quote> s3yRate(new SimpleQuote(s3yQuote));

	boost::shared_ptr<Quote> s5yRate(new SimpleQuote(s5yQuote));

	boost::shared_ptr<Quote> s10yRate(new SimpleQuote(s10yQuote));

	boost::shared_ptr<Quote> s15yRate(new SimpleQuote(s15yQuote));

	// ois

	boost::shared_ptr<Quote> ois1wRate(new SimpleQuote(ois1wQuote));

	boost::shared_ptr<Quote> ois1mRate(new SimpleQuote(ois1mQuote));

	boost::shared_ptr<Quote> ois3mRate(new SimpleQuote(ois3mQuote));

	boost::shared_ptr<Quote> ois6mRate(new SimpleQuote(ois6mQuote));

	boost::shared_ptr<Quote> ois9mRate(new SimpleQuote(ois9mQuote));

	boost::shared_ptr<Quote> ois1yRate(new SimpleQuote(ois1yQuote));

	boost::shared_ptr<Quote> ois2yRate(new SimpleQuote(ois2yQuote));

	boost::shared_ptr<Quote> ois3yRate(new SimpleQuote(ois3yQuote));

	boost::shared_ptr<Quote> ois5yRate(new SimpleQuote(ois5yQuote));

	boost::shared_ptr<Quote> ois10yRate(new SimpleQuote(ois10yQuote));

	boost::shared_ptr<Quote> ois15yRate(new SimpleQuote(ois15yQuote));

	/*********************

	***  FORWARD HELPERS ***

	*********************/



	// RateHelpers are built from the above quotes together with

	// other instrument dependant infos.  Quotes are passed in

	// relinkable handles which could be relinked to some other

	// data source later.



	// deposits

	DayCounter depositDayCounter = Actual360();



	// setup ois

	boost::shared_ptr<Eonia> eonia(new Eonia);



	boost::shared_ptr<RateHelper> ois1w(new OISRateHelper(

		2, 1 * Weeks, Handle<Quote>(ois1wRate), eonia));

	boost::shared_ptr<RateHelper> ois1m(new OISRateHelper(

		2, 1 * Months, Handle<Quote>(ois1mRate), eonia));

	boost::shared_ptr<RateHelper> ois3m(new OISRateHelper(

		2, 3 * Months, Handle<Quote>(ois3mRate), eonia));

	boost::shared_ptr<RateHelper> ois6m(new OISRateHelper(

		2, 6 * Months, Handle<Quote>(ois6mRate), eonia));

	boost::shared_ptr<RateHelper> ois9m(new OISRateHelper(

		2, 9 * Months, Handle<Quote>(ois9mRate), eonia));

	boost::shared_ptr<RateHelper> ois1y(new OISRateHelper(

		2, 1 * Years, Handle<Quote>(ois1yRate), eonia));

	boost::shared_ptr<RateHelper> ois2y(new OISRateHelper(

		2, 2 * Years, Handle<Quote>(ois2yRate), eonia));

	boost::shared_ptr<RateHelper> ois3y(new OISRateHelper(

		2, 3 * Years, Handle<Quote>(ois3yRate), eonia));

	boost::shared_ptr<RateHelper> ois5y(new OISRateHelper(

		2, 5 * Years, Handle<Quote>(ois5yRate), eonia));

	boost::shared_ptr<RateHelper> ois10y(new OISRateHelper(

		2, 10 * Years, Handle<Quote>(ois10yRate), eonia));

	boost::shared_ptr<RateHelper> ois15y(new OISRateHelper(

		2, 15 * Years, Handle<Quote>(ois15yRate), eonia));



	/*********************

	**  CURVE BUILDING **

	*********************/



	double tolerance = 1.0e-15;



	// A ois curve

	std::vector<boost::shared_ptr<RateHelper> > oisInstruments;

	oisInstruments.push_back(ois1w);

	oisInstruments.push_back(ois1m);

	oisInstruments.push_back(ois3m);

	oisInstruments.push_back(ois6m);

	oisInstruments.push_back(ois9m);

	oisInstruments.push_back(ois1y);

	oisInstruments.push_back(ois2y);

	oisInstruments.push_back(ois3y);

	oisInstruments.push_back(ois5y);

	oisInstruments.push_back(ois10y);

	oisInstruments.push_back(ois15y);

	// Any DayCounter would be fine.

	// ActualActual::ISDA ensures that 30 years is 30.0

	DayCounter termStructureDayCounter =

		ActualActual(ActualActual::ISDA);



	boost::shared_ptr<YieldTermStructure> OisTermStructure(

		new PiecewiseYieldCurve<Discount, LogLinear>(

			todaysDate,

			oisInstruments,

			termStructureDayCounter,

			tolerance));



	// Term structures that will be used for pricing:

	// the one used for discounting cash flows

	RelinkableHandle<YieldTermStructure> discountingTermStructure;

	discountingTermStructure.linkTo(OisTermStructure);


	boost::shared_ptr<RateHelper> d3m(new DepositRateHelper(

		Handle<Quote>(d3mRate),

		3 * Months, fixingDays,

		calendar, ModifiedFollowing,

		true, depositDayCounter));



	// setup FRAs

	boost::shared_ptr<RateHelper> fra3x6(new FraRateHelper(

		Handle<Quote>(fra3x6Rate),

		3, 6, fixingDays, calendar, ModifiedFollowing,

		true, depositDayCounter));

	boost::shared_ptr<RateHelper> fra6x9(new FraRateHelper(

		Handle<Quote>(fra6x9Rate),

		6, 9, fixingDays, calendar, ModifiedFollowing,

		true, depositDayCounter));

	boost::shared_ptr<RateHelper> fra6x12(new FraRateHelper(

		Handle<Quote>(fra9x12Rate),

		9, 12, fixingDays, calendar, ModifiedFollowing,

		true, depositDayCounter));





	// setup futures

	// Rate convexityAdjustment = 0.0;

	Integer futMonths = 3;

	Date imm = IMM::nextDate(settlementDate);

	boost::shared_ptr<RateHelper> fut1(new FuturesRateHelper(

		Handle<Quote>(fut1Price),

		imm,

		futMonths, calendar, ModifiedFollowing,

		true, depositDayCounter));

	imm = IMM::nextDate(imm + 1);

	boost::shared_ptr<RateHelper> fut2(new FuturesRateHelper(

		Handle<Quote>(fut2Price),

		imm,

		futMonths, calendar, ModifiedFollowing,

		true, depositDayCounter));

	imm = IMM::nextDate(imm + 1);

	boost::shared_ptr<RateHelper> fut3(new FuturesRateHelper(

		Handle<Quote>(fut3Price),

		imm,

		futMonths, calendar, ModifiedFollowing,

		true, depositDayCounter));

	imm = IMM::nextDate(imm + 1);

	boost::shared_ptr<RateHelper> fut4(new FuturesRateHelper(

		Handle<Quote>(fut4Price),

		imm,

		futMonths, calendar, ModifiedFollowing,

		true, depositDayCounter));

	imm = IMM::nextDate(imm + 1);

	boost::shared_ptr<RateHelper> fut5(new FuturesRateHelper(

		Handle<Quote>(fut5Price),

		imm,

		futMonths, calendar, ModifiedFollowing,

		true, depositDayCounter));

	imm = IMM::nextDate(imm + 1);

	boost::shared_ptr<RateHelper> fut6(new FuturesRateHelper(

		Handle<Quote>(fut6Price),

		imm,

		futMonths, calendar, ModifiedFollowing,

		true, depositDayCounter));

	imm = IMM::nextDate(imm + 1);

	boost::shared_ptr<RateHelper> fut7(new FuturesRateHelper(

		Handle<Quote>(fut7Price),

		imm,

		futMonths, calendar, ModifiedFollowing,

		true, depositDayCounter));

	imm = IMM::nextDate(imm + 1);

	boost::shared_ptr<RateHelper> fut8(new FuturesRateHelper(

		Handle<Quote>(fut8Price),

		imm,

		futMonths, calendar, ModifiedFollowing,

		true, depositDayCounter));





	// setup swaps

	Frequency swFixedLegFrequency = Annual;

	BusinessDayConvention swFixedLegConvention = Unadjusted;

	DayCounter swFixedLegDayCounter = Thirty360(Thirty360::European);

	boost::shared_ptr<IborIndex> swFloatingLegIndex(new

		IborIndex("ibor", 3 * Months, 2, EURCurrency(),

			calendar, ModifiedFollowing, true,

			depositDayCounter));



	boost::shared_ptr<RateHelper> s2y(new SwapRateHelper(

		Handle<Quote>(s2yRate), 2 * Years,

		calendar, swFixedLegFrequency,

		swFixedLegConvention, swFixedLegDayCounter,

		swFloatingLegIndex, Handle<Quote>(), 0 * Days, discountingTermStructure));

	boost::shared_ptr<RateHelper> s3y(new SwapRateHelper(

		Handle<Quote>(s3yRate), 3 * Years,

		calendar, swFixedLegFrequency,

		swFixedLegConvention, swFixedLegDayCounter,

		swFloatingLegIndex, Handle<Quote>(), 0 * Days, discountingTermStructure));

	boost::shared_ptr<RateHelper> s5y(new SwapRateHelper(

		Handle<Quote>(s5yRate), 5 * Years,

		calendar, swFixedLegFrequency,

		swFixedLegConvention, swFixedLegDayCounter,

		swFloatingLegIndex, Handle<Quote>(), 0 * Days, discountingTermStructure));

	boost::shared_ptr<RateHelper> s10y(new SwapRateHelper(

		Handle<Quote>(s10yRate), 10 * Years,

		calendar, swFixedLegFrequency,

		swFixedLegConvention, swFixedLegDayCounter,

		swFloatingLegIndex, Handle<Quote>(), 0 * Days, discountingTermStructure));

	boost::shared_ptr<RateHelper> s15y(new SwapRateHelper(

		Handle<Quote>(s15yRate), 15 * Years,

		calendar, swFixedLegFrequency,

		swFixedLegConvention, swFixedLegDayCounter,

		swFloatingLegIndex, Handle<Quote>(), 0 * Days, discountingTermStructure));



	/*********************

	**  CURVE BUILDING **

	*********************/



	// Any DayCounter would be fine.

	// ActualActual::ISDA ensures that 30 years is 30.0

	//DayCounter termStructureDayCounter =

	//    ActualActual(ActualActual::ISDA);



	// A depo-swap curve

	std::vector<boost::shared_ptr<RateHelper> > depoSwapInstruments;

	//depoSwapInstruments.push_back(d1w);

	//depoSwapInstruments.push_back(d1m);

	depoSwapInstruments.push_back(d3m);

	//depoSwapInstruments.push_back(d6m);

	//depoSwapInstruments.push_back(d9m);

	//depoSwapInstruments.push_back(d1y);

	depoSwapInstruments.push_back(s2y);

	depoSwapInstruments.push_back(s3y);

	depoSwapInstruments.push_back(s5y);

	depoSwapInstruments.push_back(s10y);

	depoSwapInstruments.push_back(s15y);

	/*
	boost::shared_ptr<ForwardRateCurve> depoSwapTermStructure(

		new FwdRateCurve<Linear>("ibor",

			3 * Months,

			2,

			EURCurrency(),

			calendar,

			ModifiedFollowing,

			true,

			depositDayCounter,

			depoSwapInstruments,

			tolerance));


*/


// A depo-futures-swap curve

	std::vector<boost::shared_ptr<RateHelper> > depoFutSwapInstruments;

	//depoFutSwapInstruments.push_back(d1w);

	//depoFutSwapInstruments.push_back(d1m);

	depoFutSwapInstruments.push_back(fut1);

	depoFutSwapInstruments.push_back(fut2);

	depoFutSwapInstruments.push_back(fut3);

	depoFutSwapInstruments.push_back(fut4);

	depoFutSwapInstruments.push_back(fut5);

	depoFutSwapInstruments.push_back(fut6);

	depoFutSwapInstruments.push_back(fut7);

	depoFutSwapInstruments.push_back(fut8);

	depoFutSwapInstruments.push_back(s3y);

	depoFutSwapInstruments.push_back(s5y);

	depoFutSwapInstruments.push_back(s10y);

	depoFutSwapInstruments.push_back(s15y);

	/*
	boost::shared_ptr<ForwardRateCurve> depoFutSwapTermStructure(

		new FwdRateCurve<Linear>("ibor",

			3 * Months,

			2,

			EURCurrency(),

			calendar,

			ModifiedFollowing,

			true,

			depositDayCounter,

			depoFutSwapInstruments,

			tolerance));
*/


// A depo-FRA-swap curve

	std::vector<boost::shared_ptr<RateHelper> > depoFRASwapInstruments;

	//depoFRASwapInstruments.push_back(d1w);

	//depoFRASwapInstruments.push_back(d1m);

	depoFRASwapInstruments.push_back(d3m);

	depoFRASwapInstruments.push_back(fra3x6);

	depoFRASwapInstruments.push_back(fra6x9);

	depoFRASwapInstruments.push_back(fra6x12);

	depoFRASwapInstruments.push_back(s2y);

	depoFRASwapInstruments.push_back(s3y);

	depoFRASwapInstruments.push_back(s5y);

	depoFRASwapInstruments.push_back(s10y);

	depoFRASwapInstruments.push_back(s15y);
	/*
	boost::shared_ptr<ForwardRateCurve> depoFRASwapTermStructure(

		new FwdRateCurve<Linear>("ibor",

			3 * Months,

			2,

			EURCurrency(),

			calendar,

			ModifiedFollowing,

			true,

			depositDayCounter,

			depoFRASwapInstruments,

			tolerance));





	// the one used for forward rate forecasting

	RelinkableHandle<ForwardRateCurve> forecastingTermStructure;

*/


/*
/*********************

* Bumped instruments for 3M *

**********************/




// deposits

	Rate d1wQuote_ = 0.04202;

	Rate d1mQuote_ = 0.04092;

	Rate d3mQuote_ = 0.03993;

	Rate d6mQuote_ = 0.03883;

	Rate d9mQuote_ = 0.03828;

	Rate d1yQuote_ = 0.03795;

	// FRAs

	Rate fra3x6Quote_ = 0.0408375;

	Rate fra6x9Quote_ = 0.0408375;

	Rate fra6x12Quote_ = 0.0408375;

	// futures

	Real fut1Quote_ = 105.91625;

	Real fut2Quote_ = 106.46625;

	Real fut3Quote_ = 106.68625;

	Real fut4Quote_ = 106.35625;

	Real fut5Quote_ = 106.13625;

	Real fut6Quote_ = 106.02625;

	Real fut7Quote_ = 105.91625;

	Real fut8Quote_ = 105.69625;

	// swaps

	Rate s2yQuote_ = 0.0408375;

	Rate s3yQuote_ = 0.04378;

	Rate s5yQuote_ = 0.04873;

	Rate s10yQuote_ = 0.056815;

	Rate s15yQuote_ = 0.0606925;



	/********************

	***    Quote_S    ***

	********************/



	// SimpleQuote stores a value which can be manually changed;

	// other Quote_ subclasses could read the value from a database

	// or some kind of data feed.



	// deposits

	boost::shared_ptr<Quote> d1wRate_(new SimpleQuote(d1wQuote_));

	boost::shared_ptr<Quote> d1mRate_(new SimpleQuote(d1mQuote_));

	boost::shared_ptr<Quote> d3mRate_(new SimpleQuote(d3mQuote_));

	boost::shared_ptr<Quote> d6mRate_(new SimpleQuote(d6mQuote_));

	boost::shared_ptr<Quote> d9mRate_(new SimpleQuote(d9mQuote_));

	boost::shared_ptr<Quote> d1yRate_(new SimpleQuote(d1yQuote_));

	// FRAs

	boost::shared_ptr<Quote> fra3x6Rate_(new SimpleQuote(fra3x6Quote_));

	boost::shared_ptr<Quote> fra6x9Rate_(new SimpleQuote(fra6x9Quote_));

	boost::shared_ptr<Quote> fra9x12Rate_(new SimpleQuote(fra6x12Quote_));

	// futures

	boost::shared_ptr<Quote> fut1Price_(new SimpleQuote(fut1Quote_));

	boost::shared_ptr<Quote> fut2Price_(new SimpleQuote(fut2Quote_));

	boost::shared_ptr<Quote> fut3Price_(new SimpleQuote(fut3Quote_));

	boost::shared_ptr<Quote> fut4Price_(new SimpleQuote(fut4Quote_));

	boost::shared_ptr<Quote> fut5Price_(new SimpleQuote(fut5Quote_));

	boost::shared_ptr<Quote> fut6Price_(new SimpleQuote(fut6Quote_));

	boost::shared_ptr<Quote> fut7Price_(new SimpleQuote(fut7Quote_));

	boost::shared_ptr<Quote> fut8Price_(new SimpleQuote(fut8Quote_));

	// swaps

	boost::shared_ptr<Quote> s2yRate_(new SimpleQuote(s2yQuote_));

	boost::shared_ptr<Quote> s3yRate_(new SimpleQuote(s3yQuote_));

	boost::shared_ptr<Quote> s5yRate_(new SimpleQuote(s5yQuote_));

	boost::shared_ptr<Quote> s10yRate_(new SimpleQuote(s10yQuote_));

	boost::shared_ptr<Quote> s15yRate_(new SimpleQuote(s15yQuote_));





	// RateHelpers are built from the above Quote_s together with

	// other instrument dependant infos.  Quote_s are passed in

	// relinkable handles which could be relinked to some other

	// data source later.



	// deposits



	/*********************

	**  CURVE BUILDING **

	*********************/




	// Term structures that will be used for pricing:



	boost::shared_ptr<RateHelper> d3m_(new DepositRateHelper(

		Handle<Quote>(d3mRate_),

		3 * Months, fixingDays,

		calendar, ModifiedFollowing,

		true, depositDayCounter));



	// setup FRAs

	boost::shared_ptr<RateHelper> fra3x6_(new FraRateHelper(

		Handle<Quote>(fra3x6Rate_),

		3, 6, fixingDays, calendar, ModifiedFollowing,

		true, depositDayCounter));

	boost::shared_ptr<RateHelper> fra6x9_(new FraRateHelper(

		Handle<Quote>(fra6x9Rate_),

		6, 9, fixingDays, calendar, ModifiedFollowing,

		true, depositDayCounter));

	boost::shared_ptr<RateHelper> fra9x12_(new FraRateHelper(

		Handle<Quote>(fra9x12Rate_),

		9, 12, fixingDays, calendar, ModifiedFollowing,

		true, depositDayCounter));


	// setup futures

	boost::shared_ptr<RateHelper> fut1_(new FuturesRateHelper(

		Handle<Quote>(fut1Price_),

		imm,

		futMonths, calendar, ModifiedFollowing,

		true, depositDayCounter));

	imm = IMM::nextDate(imm + 1);

	boost::shared_ptr<RateHelper> fut2_(new FuturesRateHelper(

		Handle<Quote>(fut2Price_),

		imm,

		futMonths, calendar, ModifiedFollowing,

		true, depositDayCounter));

	imm = IMM::nextDate(imm + 1);

	boost::shared_ptr<RateHelper> fut3_(new FuturesRateHelper(

		Handle<Quote>(fut3Price_),

		imm,

		futMonths, calendar, ModifiedFollowing,

		true, depositDayCounter));

	imm = IMM::nextDate(imm + 1);

	boost::shared_ptr<RateHelper> fut4_(new FuturesRateHelper(

		Handle<Quote>(fut4Price_),

		imm,

		futMonths, calendar, ModifiedFollowing,

		true, depositDayCounter));

	imm = IMM::nextDate(imm + 1);

	boost::shared_ptr<RateHelper> fut5_(new FuturesRateHelper(

		Handle<Quote>(fut5Price_),

		imm,

		futMonths, calendar, ModifiedFollowing,

		true, depositDayCounter));

	imm = IMM::nextDate(imm + 1);

	boost::shared_ptr<RateHelper> fut6_(new FuturesRateHelper(

		Handle<Quote>(fut6Price_),

		imm,

		futMonths, calendar, ModifiedFollowing,

		true, depositDayCounter));

	imm = IMM::nextDate(imm + 1);

	boost::shared_ptr<RateHelper> fut7_(new FuturesRateHelper(

		Handle<Quote>(fut7Price_),

		imm,

		futMonths, calendar, ModifiedFollowing,

		true, depositDayCounter));

	imm = IMM::nextDate(imm + 1);

	boost::shared_ptr<RateHelper> fut8_(new FuturesRateHelper(

		Handle<Quote>(fut8Price_),

		imm,

		futMonths, calendar, ModifiedFollowing,

		true, depositDayCounter));





	// setup swaps


	boost::shared_ptr<IborIndex> swFloatingLegIndex_(new

		IborIndex("ibor", 3 * Months, 2, EURCurrency(),

			calendar, ModifiedFollowing, true,

			depositDayCounter));



	boost::shared_ptr<RateHelper> s2y_(new SwapRateHelper(

		Handle<Quote>(s2yRate), 2 * Years,

		calendar, swFixedLegFrequency,

		swFixedLegConvention, swFixedLegDayCounter,

		swFloatingLegIndex_, Handle<Quote>(), 0 * Days, discountingTermStructure));

	boost::shared_ptr<RateHelper> s3y_(new SwapRateHelper(

		Handle<Quote>(s3yRate), 3 * Years,

		calendar, swFixedLegFrequency,

		swFixedLegConvention, swFixedLegDayCounter,

		swFloatingLegIndex_, Handle<Quote>(), 0 * Days, discountingTermStructure));

	boost::shared_ptr<RateHelper> s5y_(new SwapRateHelper(

		Handle<Quote>(s5yRate), 5 * Years,

		calendar, swFixedLegFrequency,

		swFixedLegConvention, swFixedLegDayCounter,

		swFloatingLegIndex_, Handle<Quote>(), 0 * Days, discountingTermStructure));

	boost::shared_ptr<RateHelper> s10y_(new SwapRateHelper(

		Handle<Quote>(s10yRate), 10 * Years,

		calendar, swFixedLegFrequency,

		swFixedLegConvention, swFixedLegDayCounter,

		swFloatingLegIndex_, Handle<Quote>(), 0 * Days, discountingTermStructure));

	boost::shared_ptr<RateHelper> s15y_(new SwapRateHelper(

		Handle<Quote>(s15yRate), 15 * Years,

		calendar, swFixedLegFrequency,

		swFixedLegConvention, swFixedLegDayCounter,

		swFloatingLegIndex_, Handle<Quote>(), 0 * Days, discountingTermStructure));



	/*********************

	**  CURVE BUILDING **

	*********************/



	// Any DayCounter would be fine.

	// ActualActual::ISDA ensures that 30 years is 30.0

	//DayCounter termStructureDayCounter =

	//    ActualActual(ActualActual::ISDA);



	// A depo-futures-swap curve

	std::vector<boost::shared_ptr<RateHelper> > depoFutSwapInstruments_;


	depoFutSwapInstruments_.push_back(fut1_);

	depoFutSwapInstruments_.push_back(fut2_);

	depoFutSwapInstruments_.push_back(fut3_);

	depoFutSwapInstruments_.push_back(fut4_);

	depoFutSwapInstruments_.push_back(fut5_);

	depoFutSwapInstruments_.push_back(fut6_);

	depoFutSwapInstruments_.push_back(fut7_);

	depoFutSwapInstruments_.push_back(fut8_);

	depoFutSwapInstruments_.push_back(s3y_);

	depoFutSwapInstruments_.push_back(s5y_);

	depoFutSwapInstruments_.push_back(s10y_);

	depoFutSwapInstruments_.push_back(s15y_);








	/*
	/*********************

	* SWAPS TO BE PRICED *

	**********************/

	/*
			// constant nominal 1,000,000 Euro

			Real nominal = 1000000.0;

			// fixed leg

			Frequency fixedLegFrequency = Annual;

			BusinessDayConvention fixedLegConvention = Unadjusted;

			BusinessDayConvention floatingLegConvention = ModifiedFollowing;

			DayCounter fixedLegDayCounter = Thirty360(Thirty360::European);

			Rate fixedRate = 0.04;

			DayCounter floatingLegDayCounter = Actual360();



			// floating leg

			Frequency floatingLegFrequency = Quarterly;

			boost::shared_ptr<IborIndex> euriborIndex(

				new IborIndex("ibor", 3 * Months, 2, EURCurrency(),

					calendar, ModifiedFollowing, true,

					depositDayCounter,

					forecastingTermStructure));

			Spread spread = 0.0;



			Integer lenghtInYears = 5;

			VanillaSwap::Type swapType = VanillaSwap::Payer;



			Date maturity = settlementDate + lenghtInYears * Years;

			Schedule fixedSchedule(settlementDate, maturity,

				Period(fixedLegFrequency),

				calendar, fixedLegConvention,

				fixedLegConvention,

				DateGeneration::Forward, false);

			Schedule floatSchedule(settlementDate, maturity,

				Period(floatingLegFrequency),

				calendar, floatingLegConvention,

				floatingLegConvention,

				DateGeneration::Forward, false);

			VanillaSwap spot5YearSwap(swapType, nominal,

				fixedSchedule, fixedRate, fixedLegDayCounter,

				floatSchedule, euriborIndex, spread,

				floatingLegDayCounter);



			Date fwdStart = calendar.advance(settlementDate, 1, Years);

			Date fwdMaturity = fwdStart + lenghtInYears * Years;

			Schedule fwdFixedSchedule(fwdStart, fwdMaturity,

				Period(fixedLegFrequency),

				calendar, fixedLegConvention,

				fixedLegConvention,

				DateGeneration::Forward, false);

			Schedule fwdFloatSchedule(fwdStart, fwdMaturity,

				Period(floatingLegFrequency),

				calendar, floatingLegConvention,

				floatingLegConvention,

				DateGeneration::Forward, false);

			VanillaSwap oneYearForward5YearSwap(swapType, nominal,

				fwdFixedSchedule, fixedRate, fixedLegDayCounter,

				fwdFloatSchedule, euriborIndex, spread,

				floatingLegDayCounter);
	*/

	/*********************

	* euriborTermStructure *

	**********************/

	RelinkableHandle<YieldTermStructure> euriborTermStructure3M;
	boost::shared_ptr<IborIndex> euriborIndex3M(
		new Euribor3M(euriborTermStructure3M));

	RelinkableHandle<YieldTermStructure> euriborTermStructure6M;
	boost::shared_ptr<IborIndex> euriborIndex6M(
		new Euribor3M(euriborTermStructure6M));
	/*********************

	* acdt-abcd Test *

	**********************/

	/*********************

	* acdt 1  EUR3M VS Eonia*

	**********************/

	//acdt1 coefficients
	std::vector<Real> coeffsacdt1(4);

	coeffsacdt1[0] = 0.001;

	coeffsacdt1[1] = 0.0019;

	coeffsacdt1[2] = 0.002;

	coeffsacdt1[3] = 2;

	//implied b from acdt

	Real Implied_b = b(coeffsacdt1);

	//OptimizationMethod:Levenberg

	boost::shared_ptr<OptimizationMethod> Lev(new LevenbergMarquardt());
	/*LevenbergMarquardt Levi;
	OptimizationMethod * Lev = & Levi;*/
	//acdt :built as EUR3M VS Eonia	
	//acdt building with and without pointer: just because we saw problem in previous test

	// as object

	boost::shared_ptr<TenorBasis> tenorBasisAcdt1(
		new AcdtTenorBasis(euriborIndex3M,

			boost::shared_ptr<Eonia>(

				new Eonia(discountingTermStructure)),

			settlementDate,

			false,

			coeffsacdt1));

	//as pointer

	AcdtTenorBasis tenorBasisAcdt1_(euriborIndex3M,

		boost::shared_ptr<Eonia>(

			new Eonia(discountingTermStructure)),

		settlementDate,

		false,

		coeffsacdt1);

	//calibration parameters

	//weights of RateHelpers errors

	std::vector<QuantLib::Real> w;
	Integer size = depoFRASwapInstruments.size();
	w.resize(size);
	for (Integer i = 0;i < size;i++) {
		w[i] = 1.0;
	}

	//EndCriteria

	Size maxIterations = 1000;
	Size maxStationaryStateIterations = 100;
	Real rootEpsilon = 0.00000001;
	Real functionEpsilon = 0.00003;
	Real gradientNormEpsilon = 0.00003;
	EndCriteria endCriteria(maxIterations,
		maxStationaryStateIterations,
		rootEpsilon,
		functionEpsilon,
		gradientNormEpsilon);

	//boolean value for fixinf parameters

	std::vector<bool> fixParaAcdt;
	fixParaAcdt.push_back(false);
	fixParaAcdt.push_back(false);
	fixParaAcdt.push_back(false);
	fixParaAcdt.push_back(false);

	//calibrate acdt 

	/*std::cout << "depoFRASwapInstruments.size()=" << depoFRASwapInstruments.size() << std::endl;
	std::cout << "w.size()=" << w.size() << std::endl;
	std::cout << "fixParaAcdt.size()=" << fixParaAcdt.size() << std::endl;*/


	tenorBasisAcdt1_.calibrate(depoFRASwapInstruments, *Lev, endCriteria, w, fixParaAcdt);
	tenorBasisAcdt1->calibrate(depoFRASwapInstruments, *Lev, endCriteria, w, fixParaAcdt);

	/*********************

	* acdt 2  EUR6M VS Eonia*

	**********************/

	std::vector<Real> coeffsacdt2(4);

	coeffsacdt2[0] = coeffsacdt1[0] + 0.1;

	coeffsacdt2[1] = coeffsacdt1[1] + 0.1;

	coeffsacdt2[2] = coeffsacdt1[2] + 0.1;

	coeffsacdt2[3] = coeffsacdt1[3] + 1.5;

	boost::shared_ptr<TenorBasis> tenorBasisAcdt2(
		new AcdtTenorBasis(euriborIndex3M,

			boost::shared_ptr<Eonia>(

				new Eonia(discountingTermStructure)),

			settlementDate,

			false,

			coeffsacdt2));

	//calibrate Acdt 2

	//weights
	std::vector<QuantLib::Real> w2;
	Integer size2 = depoFutSwapInstruments_.size();
	w2.resize(size2);
	for (Integer i = 0;i < size2;i++) {
		w2[i] = 1.0;
	}

	tenorBasisAcdt2->calibrate(depoFutSwapInstruments_, *Lev, endCriteria, w2, fixParaAcdt);

	std::vector<Real> acdt2;
	acdt2 = tenorBasisAcdt2->instCoefficients();

	/*********************

	* abcd 1  EUR3M VS Eonia*

	**********************/

	//abcd coefficients

	std::vector<Real> coeffs(4);

	coeffs[0] = coeffsacdt1[0];

	coeffs[1] = Implied_b;//equal guess

	coeffs[2] = coeffsacdt1[1];

	coeffs[3] = coeffsacdt1[2];


	//Abcd building w & wo pointer
	boost::shared_ptr<TenorBasis> tenorBasisAbcd(
		new AbcdTenorBasis(euriborIndex3M,

			boost::shared_ptr<Eonia>(

				new Eonia(discountingTermStructure)),

			settlementDate,

			false,

			coeffs));

	AbcdTenorBasis tenorBasisAbcd_(euriborIndex3M,

		boost::shared_ptr<Eonia>(

			new Eonia(discountingTermStructure)),

		settlementDate,

		false,

		coeffs);

	//calibration parameters
	std::vector<bool> fixParaAbcd;
	fixParaAbcd.push_back(false);
	fixParaAbcd.push_back(false);
	fixParaAbcd.push_back(false);
	fixParaAbcd.push_back(false);

	//calibrate
	tenorBasisAbcd->calibrate(depoFRASwapInstruments, *Lev, endCriteria, w, fixParaAbcd);
	tenorBasisAbcd_.calibrate(depoFRASwapInstruments, *Lev, endCriteria, w, fixParaAbcd);

	//consistency test

	std::vector<Real> acdt;
	std::vector<Real> abcd;
	std::vector<Real> acdt_P;

	//inst basis coefficients
	acdt = tenorBasisAcdt1_.instCoefficients();
	abcd = tenorBasisAbcd->instCoefficients();
	acdt_P = tenorBasisAcdt1->instCoefficients();

	// retrieving t_max
	Real t_1 = t(acdt);
	Real t_2 = t(abcd);
	Real t_3 = t(acdt_P);



	Date d1 = tenorBasisAcdt1_.maximumLocation();
	Date d2 = tenorBasisAbcd_.maximumLocation();
	Real t_1_ = tenorBasisAcdt1_.timeFromReference(d1);
	Real t_2_ = tenorBasisAbcd_.timeFromReference(d2);

	std::cout << "Consistency Test abcd VS acdt" << std::endl;std::cout << std::endl;

	std::cout << "Guessed acdt" << std::endl;std::cout << std::endl;

	std::cout << "a=" << coeffsacdt1[0] << std::endl;
	std::cout << "c=" << coeffsacdt1[1] << std::endl;
	std::cout << "d=" << coeffsacdt1[2] << std::endl;
	std::cout << "t=" << coeffsacdt1[3] << std::endl;
	std::cout << "Implied b=" << Implied_b << std::endl;std::cout << std::endl;

	std::cout << "Calibrated acdt" << std::endl;std::cout << std::endl;
	std::cout << "a=" << acdt[0] << std::endl;
	std::cout << "b=" << acdt[1] << std::endl;
	std::cout << "c=" << acdt[2] << std::endl;
	std::cout << "d=" << acdt[3] << std::endl;
	std::cout << "Implied t=" << t_1 << std::endl;
	std::cout << "Model t=" << t_1_ << std::endl;
	std::cout << "error=" << error(tenorBasisAcdt1_.problemValues()) << std::endl;

	std::cout << std::endl;

	std::cout << "Calibrated acdt pointer " << std::endl;std::cout << std::endl;

	std::cout << "a=" << acdt_P[0] << std::endl;
	std::cout << "b=" << acdt_P[1] << std::endl;
	std::cout << "c=" << acdt_P[2] << std::endl;
	std::cout << "d=" << acdt_P[3] << std::endl;
	std::cout << "Implied t=" << t_3 << std::endl;
	std::cout << "Model t=" << "Not available, because TenorBasis has no maximumLocation()" << std::endl;
	std::cout << "error=" << error(tenorBasisAcdt1->problemValues()) << std::endl;

	std::cout << std::endl;

	std::cout << "If the guessed coefficients are the same, the previous outputs should be the same" << std::endl;

	std::cout << std::endl;

	std::cout << "Guessed abcd" << std::endl;

	std::cout << std::endl;

	std::cout << "a=" << coeffs[0] << std::endl;
	std::cout << "b=" << coeffs[1] << std::endl;
	std::cout << "c=" << coeffs[2] << std::endl;
	std::cout << "d=" << coeffs[3] << std::endl;
	std::cout << "Implied t=" << t(coeffs) << std::endl;std::cout << std::endl;


	std::cout << "Calibrated abcd" << std::endl;std::cout << std::endl;

	std::cout << "a=" << abcd[0] << std::endl;
	std::cout << "b=" << abcd[1] << std::endl;
	std::cout << "c=" << abcd[2] << std::endl;
	std::cout << "d=" << abcd[3] << std::endl;
	std::cout << "Implied t=" << t_2 << std::endl;
	std::cout << "Model t=" << t_2_ << std::endl;
	std::cout << "error=" << error(tenorBasisAbcd->problemValues()) << std::endl;

	std::cout << std::endl;

	std::cout << "Acdt and abcd parameters should be different because of optimization method impact that differs from t to b" << std::endl;

	std::cout << std::endl;

	std::cout << "Parameter conversion" << std::endl; std::cout << std::endl;

	std::vector<Real> adst;
	adst.push_back(0.00004417701262085);
	adst.push_back(0.001300);
	adst.push_back(0.001742);
	adst.push_back(1.803269756);
	std::cout << "a=" << adst[0] << std::endl;
	std::cout << "d=" << adst[1] << std::endl;
	std::cout << "s=" << adst[2] << std::endl;
	std::cout << "t=" << adst[3] << std::endl;

	std::vector<Real> adstToAcdt;
	std::vector<Real> guess;
	guess.push_back(0.213381);
	Real acc= 0.000000000000001;
	Real max=1;
	Real min=-1;
	adstToAcdt = tenorBasisAcdt1->parameterConversion(adst,guess,acc,min,max);

	std::cout << "a=" << adstToAcdt[0] << std::endl;
	std::cout << "c=" << adstToAcdt[1] << std::endl;
	std::cout << "d=" << adstToAcdt[2] << std::endl;
	std::cout << "t=" << adstToAcdt[3] << std::endl;
	std::cout << "c guess=" << abcd[2] << std::endl;





	std::cout << std::endl;

	/*********************

	* Global Helpers *

	**********************/

	//weights
	std::vector<QuantLib::Real> weights;
	weights.resize(size-1);
	for (Integer i = 0;i < size-1;i++) {
		weights[i] = 1.0;
	}

	//fixpara for helpers calibration

	std::vector<bool> fixPara1;
	std::vector<bool> fixPara2;
	//std::vector<bool> fixPara3;

	//fixing a for each model

	//a fixing
	fixPara1.push_back(true);//fixing a
	fixPara1.push_back(false);
	fixPara1.push_back(false);
	fixPara1.push_back(false);

	fixPara2.push_back(true);//fixing a
	fixPara2.push_back(false);
	fixPara2.push_back(false);
	fixPara2.push_back(false);

	//c fixing
	/*fixPara1.push_back(false);
	fixPara1.push_back(true);//fixing c
	fixPara1.push_back(false);
	fixPara1.push_back(false);

	fixPara2.push_back(false);
	fixPara2.push_back(true);//fixing c
	fixPara2.push_back(false);
	fixPara2.push_back(false);*/

	//Set the same value of a 
	Array params1 = tenorBasisAcdt1->params(); //get parameters
	Array params2 = tenorBasisAcdt2->params(); //get parameters

	//a
	params2[0] = params1[0];

	//c
	//params2[1] = params1[1];

	tenorBasisAcdt2->setParams(params2);
	//Uodate the guess, I dont't like this chaos that I've made declaring  a lot of variables. I'm sorry, I'll improve it
	t_3 = t(acdt2);
	//a
	acdt2[0] = params2[0];
	//c
	//acdt2[1] = params2[1];


	/*fixPara3.push_back(false);// not a problem to fix abcd too, because a it's always the first parameter
	fixPara3.push_back(false);
	fixPara3.push_back(false);
	fixPara3.push_back(false);*/

	//Optimization method

	boost::shared_ptr<OptimizationMethod> Lev1(new LevenbergMarquardt());
	boost::shared_ptr<OptimizationMethod> Lev2(new LevenbergMarquardt());
	//boost::shared_ptr<OptimizationMethod> Lev3(new LevenbergMarquardt());


	//acdt and abcd global helpers

	boost::shared_ptr<GlobalHelper> globalHelper1(new GlobalHelper(tenorBasisAcdt1, depoFRASwapInstruments, Lev1, endCriteria, weights, fixPara1));
	boost::shared_ptr<GlobalHelper> globalHelper2(new GlobalHelper(tenorBasisAcdt2, depoFutSwapInstruments_, Lev2, endCriteria, w2, fixPara2));
	//boost::shared_ptr<GlobalHelper> globalHelper3(new GlobalHelper(tenorBasisAbcd, depoFutSwapInstruments_, Lev3, endCriteria, w2, fixPara3));


	//vector of GH

	std::vector<boost::shared_ptr<GlobalHelper>> helperVec;
	helperVec.push_back(globalHelper2);
	helperVec.push_back(globalHelper1);

	//helperVec.push_back(globalHelper3);


	/*********************

	* Global Model *

	**********************/



	//global model parameters

	//weights

	std::vector<QuantLib::Real> weightsTot;
	//int size1 = helperVec.size();
	weightsTot.push_back(1.0);
	weightsTot.push_back(1.0);
	//weightsTot.push_back(1.0);

	//Number of parameters: 1 (just 1 )

	QuantLib::Size s = 1;

	//Guess
	//a
	std::vector<Real> coef(1);
	coef[0] = params2[0];//a guess, will be changed from the external model and internally fixed
	//c
	

	//Position to manage: because we are fixing just 1 value the length of position is 1, and because the parameters is the first the value asisgned is 0

	std::vector<QuantLib::Integer> position;
	position.push_back(0);//a
	//position.push_back(1);//c

	//Number of inner global error: just one because we are fixing  just 1 

	//the parameter needs to be changed by the model, therefore: false
	std::vector<bool> fixPara;
	fixPara.push_back(false);

	//optimization method

	//boost::shared_ptr<OptimizationMethod> Sim(new Simplex(coef));
	boost::shared_ptr<OptimizationMethod> LevTot(new LevenbergMarquardt());


	//building
	GlobalModel GM(s, coef, helperVec, position);



	/*********************

	* Single parameter fixing : a *

	**********************/

	//save guesses
	params1 = tenorBasisAcdt1->params(); //get parameters
	params2 = tenorBasisAcdt2->params(); //get parameters
	std::vector<Real> fff;
	//calibration
	//GM.calibrate(helperVec, *LevTot, endCriteria, weightsTot, fixPara);
	GM.calibrate(*LevTot, endCriteria,fff, fixPara);
	//outputs

	std::vector<Real> acdt1_;
	std::vector<Real> acdt2_;
	//std::vector<Real> abcd_;

	//inst basis coefficients

	acdt1_ = tenorBasisAcdt1->instCoefficients();
	acdt2_ = tenorBasisAcdt2->instCoefficients();
	//abcd_ = tenorBasisAbcd->instCoefficients();
	// retrieving t_max ( note: also for acdt basis coefficients are abcd)




	std::cout << "'a' fixing ( using the output of the previous calibration as guesses  and EUR3M vs Eonia as 'guessed a')" << std::endl;std::cout << std::endl;

	std::cout << "Guessed acdt EUR3M vs Eonia " << std::endl;std::cout << std::endl;

	std::cout << "a=" << acdt[0] << std::endl;
	std::cout << "c=" << acdt[2] << std::endl;
	std::cout << "d=" << acdt[3] << std::endl;
	std::cout << "t=" << t_1 << std::endl;
	std::cout << "Implied b=" << acdt[1] << std::endl;std::cout << std::endl;

	std::cout << std::endl;

	std::cout << "Guessed acdt EUR3M+ vs Eonia " << std::endl;std::cout << std::endl;

	std::cout << std::endl;

	std::cout << "a=" << acdt2[0] << std::endl;
	std::cout << "c=" << acdt2[2] << std::endl;
	std::cout << "d=" << acdt2[3] << std::endl;
	std::cout << "t=" << t_3 << std::endl;
	std::cout << "Implied b=" << b(acdt2) << std::endl;

	std::cout << std::endl;

	/*std::cout << "Guessed abcd EUR3M vs Eonia" << std::endl;std::cout << std::endl;

	std::cout << std::endl;

	std::cout << "a=" << abcd[0] << std::endl;
	std::cout << "c=" << abcd[2] << std::endl;
	std::cout << "d=" << abcd[3] << std::endl;
	std::cout << "t=" << t_2 << std::endl;
	std::cout << "Implied b=" << Implied_b << std::endl;std::cout << std::endl;

	std::cout << std::endl;*/

	t_1 = t(acdt1_);
	t_3 = t(acdt2_);
	//t_2 = t(abcd_);

	std::cout << "Global Model" << std::endl;std::cout << std::endl;

	std::cout << "Calibrated acdt EUR3M vs Eonia " << std::endl;std::cout << std::endl;
	std::cout << "a=" << acdt1_[0] << std::endl;
	std::cout << "b=" << acdt1_[1] << std::endl;
	std::cout << "c=" << acdt1_[2] << std::endl;
	std::cout << "d=" << acdt1_[3] << std::endl;
	std::cout << "Implied t=" << t_1 << std::endl;
	std::cout << "error=" << error(tenorBasisAcdt1->problemValues()) << std::endl;

	std::cout << std::endl;

	std::cout << "Calibrated acdt EUR3M+ vs Eonia " << std::endl;std::cout << std::endl;

	std::cout << "a=" << acdt2_[0] << std::endl;
	std::cout << "b=" << acdt2_[1] << std::endl;
	std::cout << "c=" << acdt2_[2] << std::endl;
	std::cout << "d=" << acdt2_[3] << std::endl;
	std::cout << "Implied t=" << t_3 << std::endl;
	std::cout << "error=" << error(tenorBasisAcdt2->problemValues()) << std::endl;

	std::cout << std::endl;


	/*********************

	* Global Error*

	Real accuracy = 0.00001;
	Real min = -0.5;//should we make these variables? These bounds are quite random
	Real max = 0.5;


	//set same guess as with the global model
	tenorBasisAcdt1->setParams(params1);
	tenorBasisAcdt2->setParams(params2);

	std::cout << "a=" << helperVec[0]->calibratedModel_->params() << std::endl;
	std::cout << "b=" << helperVec[1]->calibratedModel_->params() << std::endl;

	GlobalError GE(helperVec,
		position,
		innerError,
		accuracy,
		min,
		max);

	//set solver and it solves
	Real guess = params1[position[0]];
	Brent solver;
	Real Result=solver.template solve<GlobalError>(GE, accuracy, guess, min, max);


	// get the outputs
	acdt1_ = tenorBasisAcdt1->instCoefficients();
	acdt2_ = tenorBasisAcdt2->instCoefficients();

	std::cout << "Solver" << std::endl;std::cout << std::endl;

	std::cout << "Calibrated acdt EUR3M vs Eonia " << std::endl;std::cout << std::endl;
	std::cout << "a=" << acdt1_[0] << std::endl;
	std::cout << "b=" << acdt1_[1] << std::endl;
	std::cout << "c=" << acdt1_[2] << std::endl;
	std::cout << "d=" << acdt1_[3] << std::endl;
	std::cout << "Implied t=" << t_1 << std::endl;
	std::cout << "error=" << error(tenorBasisAcdt1->problemValues()) << std::endl;

	std::cout << std::endl;

	std::cout << "Calibrated acdt EUR3M+ vs Eonia " << std::endl;std::cout << std::endl;

	std::cout << "a=" << acdt2_[0] << std::endl;
	std::cout << "b=" << acdt2_[1] << std::endl;
	std::cout << "c=" << acdt2_[2] << std::endl;
	std::cout << "d=" << acdt2_[3] << std::endl;
	std::cout << "Implied t=" << t_3 << std::endl;
	std::cout << "error=" << error(tenorBasisAcdt2->problemValues()) << std::endl;

	std::cout << std::endl;

	std::cout << "Calibrated abcd EUR3M vs Eonia" << std::endl;std::cout << std::endl;

	std::cout << "a=" << abcd_[0] << std::endl;
	std::cout << "b=" << abcd_[1] << std::endl;
	std::cout << "c=" << abcd_[2] << std::endl;
	std::cout << "d=" << abcd_[3] << std::endl;
	std::cout << "Implied t=" << t_2 << std::endl;

	std::cout << std::endl;

	std::cout << "'a' is the same for each model" << std::endl;

	std::cout << std::endl;
	*/

	/*********************

	* Double parameter fixing : a and c *

	**********************/


	//acdt and abcd global helpers
	fixPara1.at(1) = true;//c
	fixPara2.at(1) = true;//c

	//set same c ( a already set)
	params1 = tenorBasisAcdt1->params(); //get parameters
	params2 = tenorBasisAcdt2->params(); //get parameters

	params2[1] = params1[1];

	tenorBasisAcdt2->setParams(params2);
	/*std::cout << params1 << std::endl;
	std::cout << params2 << std::endl;*/
	acdt2_[2] = params2[1];
	acdt2_[1] = b(acdt2_);


	//depoFutSwapInstruments_ w2
	boost::shared_ptr<GlobalHelper> globalHelper1_(new GlobalHelper(tenorBasisAcdt1, depoFRASwapInstruments, Lev1, endCriteria, weights, fixPara1));
	boost::shared_ptr<GlobalHelper> globalHelper2_(new GlobalHelper(tenorBasisAcdt2, depoFutSwapInstruments_, Lev2, endCriteria, w2, fixPara2));


	//vector of GH

	std::vector<boost::shared_ptr<GlobalHelper>> helperVec_;
	helperVec_.push_back(globalHelper1_);
	helperVec_.push_back(globalHelper2_);
	//helperVec.push_back(globalHelper3);

	//global model parameters


	//Number of parameters: 1

	s = 2;

	//Guess

	coef.push_back(acdt1_[0]);//a guess, will be changed from the external model and internally fixed
	//Note: c guess has been fixed in the acdtmodel and will be the selected one up to the inner calibrator will change it.

	//Position: add a new position for c

	position.at(0) = 0;
	position.push_back(1);//c

	//double errror

	//boost::shared_ptr<GlobalModel> GM(new GlobalModel(s, coef, helperVec_, position, innerError, LevTot, endCriteria, weightsTot, fixPara));
	fixPara.push_back(false);
	GlobalModel GM2(s, coef, helperVec_, position);


	// retrieving t_max ( note: also for acdt basis coefficients are abcd)


	std::cout << "'a and c' fixing ( using the output of the previous calibration as guesses )" << std::endl;std::cout << std::endl;

	std::cout << "Guessed acdt EUR3M vs Eonia " << std::endl;std::cout << std::endl;

	std::cout << "a=" << acdt1_[0] << std::endl;
	std::cout << "c=" << acdt1_[2] << std::endl;
	std::cout << "d=" << acdt1_[3] << std::endl;
	std::cout << "t=" << t_1 << std::endl;
	std::cout << "Implied b=" << acdt1_[1] << std::endl;std::cout << std::endl;

	std::cout << std::endl;

	std::cout << "Guessed acdt EUR3M+ vs Eonia " << std::endl;std::cout << std::endl;

	std::cout << std::endl;

	std::cout << "a=" << acdt2_[0] << std::endl;
	std::cout << "c=" << acdt2_[2] << std::endl;
	std::cout << "d=" << acdt2_[3] << std::endl;
	std::cout << "t=" << t_3 << std::endl;
	std::cout << "Implied b=" << acdt2_[1] << std::endl;std::cout << std::endl;
	std::cout << std::endl;

	//calibration
	//GM2.calibrate(helperVec, *LevTot, endCriteria, weightsTot, fixPara);
	GM2.calibrate(*LevTot, endCriteria, weightsTot, fixPara);
	//inst basis coefficients
	acdt1_ = tenorBasisAcdt1->instCoefficients();
	acdt2_ = tenorBasisAcdt2->instCoefficients();


	t_1 = t(acdt1_);
	t_3 = t(acdt2_);
	//t_2 =t(abcd_);


	std::cout << "Calibrated acdt EUR3M vs Eonia" << std::endl;std::cout << std::endl;
	std::cout << "a=" << acdt1_[0] << std::endl;
	std::cout << "b=" << acdt1_[1] << std::endl;
	std::cout << "c=" << acdt1_[2] << std::endl;
	std::cout << "d=" << acdt1_[3] << std::endl;
	std::cout << "Implied t=" << t_1 << std::endl;
	std::cout << "error=" << error(tenorBasisAcdt1->problemValues()) << std::endl;
	std::cout << std::endl;

	std::cout << "Calibrated acdt EUR3M+ vs Eonia " << std::endl;std::cout << std::endl;

	std::cout << "a=" << acdt2_[0] << std::endl;
	std::cout << "b=" << acdt2_[1] << std::endl;
	std::cout << "c=" << acdt2_[2] << std::endl;
	std::cout << "d=" << acdt2_[3] << std::endl;
	std::cout << "Implied t=" << t_3 << std::endl;
	std::cout << "error=" << error(tenorBasisAcdt2->problemValues()) << std::endl;

	std::cout << std::endl;


	std::cout << " 'a and c ' are the same for each model" << std::endl;

	std::cout << std::endl;



	system("pause");

	return 0;
};

