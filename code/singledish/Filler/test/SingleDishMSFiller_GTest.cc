/*
 * SingleDishMSFiller_Gtest.cc
 *
 *  Created on: Jan 8, 2016
 *      Author: nakazato
 */

#include <gtest/gtest.h>

#include <singledish/Filler/SingleDishMSFiller.h>
#include <singledish/Filler/ReaderInterface.h>
#include <singledish/Filler/Scantable2MSReader.h>
#include "TestReader.h"

#include <string>
#include <iostream>
#include <memory>

#include <casacore/casa/OS/Time.h>
#include <casacore/casa/Arrays/ArrayLogical.h>
#include <casacore/casa/Quanta/MVPosition.h>
#include <casacore/measures/Measures/MPosition.h>
#include <casacore/ms/MeasurementSets/MSObsColumns.h>
#include <casacore/ms/MeasurementSets/MSProcessorColumns.h>
#include <casacore/ms/MeasurementSets/MSAntennaColumns.h>
#include <casacore/ms/MeasurementSets/MSSourceColumns.h>
#include <casacore/ms/MeasurementSets/MSSpWindowColumns.h>
#include <casacore/ms/MeasurementSets/MSSysCalColumns.h>
#include <casacore/ms/MeasurementSets/MSWeatherColumns.h>

using namespace casa;
using namespace casacore;

#define POST_START std::cout << "Start " << __PRETTY_FUNCTION__ << std::endl
#define POST_END std::cout << "End " << __PRETTY_FUNCTION__ << std::endl

#define CASA_EXPECT_STREQ(expected, actual) EXPECT_STREQ((expected).c_str(), (actual).c_str())
#define CASA_ASSERT_STREQ(expected, actual) ASSERT_STREQ((expected).c_str(), (actual).c_str())
#define CASA_EXPECT_ARRAYEQ(expected, actual) \
  EXPECT_TRUE((expected).conform((actual))); \
  if (!(expected).empty()) { \
    EXPECT_TRUE(allEQ((expected), (actual))); \
  }

class SingleDishMSFillerTest: public ::testing::Test {
public:
  virtual void SetUp() {
    std::cout << "this is SetUp" << std::endl;

    my_ms_name_ = "mytest.ms";
    my_data_name_ = "mytest.asap";

    deleteTable(my_ms_name_);
  }

  virtual void TearDown() {
    std::cout << "this is TearDown" << std::endl;

    //deleteTable(my_ms_name_);
  }

protected:
  std::string my_ms_name_;
  std::string my_data_name_;

private:
  void deleteTable(std::string const &name) {
    File file(name);
    if (file.exists()) {
      std::cout << "Removing " << name << std::endl;
      Table::deleteTable(name, True);
    }
  }
};

TEST_F(SingleDishMSFillerTest, FillerTestWithReaderStub) {
  SingleDishMSFiller<TestReader<FloatDataStorage> > filler(my_data_name_);

  std::string const &data_name = filler.getDataName();

  EXPECT_STREQ(data_name.c_str(), my_data_name_.c_str());

  // fill MS
  filler.fill();

  // save MS
  filler.save(my_ms_name_);

  // file existence check
  ASSERT_PRED1([](std::string const &name) {
    File file(name);
    return file.exists();
  }, my_ms_name_);

  // verify table contents
  MeasurementSet myms(my_ms_name_);
  auto const &reader = filler.getReader();

  // verify OBSERVATION table
  {
    std::cout << "Verify OBSERVATION table" << std::endl;
    auto const mytable = myms.observation();
    ASSERT_EQ(uInt(1), mytable.nrow());
    TableRecord const &expected_record = reader.observation_record_;
    ROMSObservationColumns mycolumns(mytable);
    CASA_EXPECT_STREQ(expected_record.asString("OBSERVER"),
        mycolumns.observer()(0));
    CASA_EXPECT_STREQ(expected_record.asString("PROJECT"),
        mycolumns.project()(0));
    CASA_EXPECT_STREQ(expected_record.asString("TELESCOPE_NAME"),
        mycolumns.telescopeName()(0));
    CASA_EXPECT_STREQ(expected_record.asString("OBSERVER"),
        mycolumns.observer()(0));
    CASA_EXPECT_STREQ(expected_record.asString("SCHEDULE_TYPE"),
        mycolumns.scheduleType()(0));
    EXPECT_EQ(expected_record.asDouble("RELEASE_DATE"),
        mycolumns.releaseDate()(0));
    EXPECT_TRUE(
        allEQ(expected_record.asArrayDouble("TIME_RANGE"),
            mycolumns.timeRange()(0)));
    Vector < String > mylog = mycolumns.log()(0);
    EXPECT_EQ(size_t(1), mylog.size());
    CASA_EXPECT_STREQ(expected_record.asArrayString("LOG")(IPosition(1, 0)),
        mylog[0]);
    Vector < String > myschedule = mycolumns.schedule()(0);
    EXPECT_EQ(size_t(1), myschedule.size());
    CASA_EXPECT_STREQ(
        expected_record.asArrayString("SCHEDULE")(IPosition(1, 0)),
        myschedule[0]);
  }

  // verify PROCESSOR table
  {
    std::cout << "Verify PROCESSOR table" << std::endl;
    auto const mytable = myms.processor();
    ASSERT_EQ(uInt(1), mytable.nrow());
    TableRecord const &expected_record = reader.processor_record_;
    ROMSProcessorColumns mycolumns(mytable);
    CASA_EXPECT_STREQ(expected_record.asString("TYPE"), mycolumns.type()(0));
    CASA_EXPECT_STREQ(expected_record.asString("SUB_TYPE"),
        mycolumns.subType()(0));
    EXPECT_EQ(expected_record.asInt("TYPE_ID"), mycolumns.typeId()(0));
    EXPECT_EQ(expected_record.asInt("MODE_ID"), mycolumns.modeId()(0));
  }

  // verify ANTENNA table
  {
    std::cout << "Verify ANTENNA table" << std::endl;
    auto const mytable = myms.antenna();
    ASSERT_EQ(uInt(1), mytable.nrow());
    TableRecord const &expected_record = reader.antenna_record_;
    ROMSAntennaColumns mycolumns(mytable);
    CASA_EXPECT_STREQ(expected_record.asString("NAME"), mycolumns.name()(0));
    CASA_EXPECT_STREQ(expected_record.asString("STATION"),
        mycolumns.station()(0));
    CASA_EXPECT_STREQ(expected_record.asString("TYPE"), mycolumns.type()(0));
    CASA_EXPECT_STREQ(expected_record.asString("MOUNT"), mycolumns.mount()(0));
    auto const position_meas_column = mycolumns.positionMeas();
    auto const position = position_meas_column(0);
    auto const position_val = position.get("m");
    CASA_EXPECT_STREQ(String("ITRF"), position.getRefString());
    EXPECT_TRUE(
        allEQ(Vector < Double > (expected_record.asArrayDouble("POSITION")),
            position_val.getValue()));
    auto const offset_meas_column = mycolumns.offsetMeas();
    auto const offset = offset_meas_column(0);
    auto const offset_val = offset.get("m");
    CASA_EXPECT_STREQ(String("ITRF"), offset.getRefString());
    EXPECT_TRUE(
        allEQ(expected_record.asArrayDouble("OFFSET"), offset_val.getValue()));
    EXPECT_EQ(expected_record.asDouble("DISH_DIAMETER"),
        mycolumns.dishDiameter()(0));
  }

  // verify SOURCE table
  {
    std::cout << "verify SOURCE table" << std::endl;
    auto const mytable = myms.source();
    TableRecord const &expected_record = reader.source_record_;
    uInt expected_nrow = expected_record.nfields();
    ASSERT_EQ(expected_nrow, mytable.nrow());
    ROMSSourceColumns mycolumns(mytable);
    for (uInt i = 0; i < expected_nrow; ++i) {
      std::cout << "Verifying row " << i << std::endl;
      String key = "ROW" + String::toString(i);
      TableRecord row_record = expected_record.asRecord(key);
      EXPECT_EQ(row_record.asInt("SOURCE_ID"), mycolumns.sourceId()(i));
      CASA_EXPECT_STREQ(row_record.asString("NAME"), mycolumns.name()(i));
      EXPECT_EQ(row_record.asDouble("TIME"), mycolumns.time()(i));
      EXPECT_EQ(row_record.asDouble("INTERVAL"), mycolumns.interval()(i));
      EXPECT_EQ(row_record.asInt("SPECTRAL_WINDOW_ID"),
          mycolumns.spectralWindowId()(i));
      EXPECT_EQ(row_record.asInt("CALIBRATION_GROUP"),
          mycolumns.calibrationGroup()(i));
      CASA_EXPECT_STREQ(row_record.asString("CODE"), mycolumns.code()(i));
      EXPECT_EQ(row_record.asInt("NUM_LINES"), mycolumns.numLines()(i));
      Array<Double> expected_array = row_record.asArrayDouble("DIRECTION");
      Array<Double> array = mycolumns.direction()(i);
      CASA_EXPECT_ARRAYEQ(expected_array, array);
      expected_array.assign(row_record.asArrayDouble("PROPER_MOTION"));
      array.assign(mycolumns.properMotion()(i));
      CASA_EXPECT_ARRAYEQ(expected_array, array);
      expected_array.assign(row_record.asArrayDouble("REST_FREQUENCY"));
      array.assign(mycolumns.restFrequency()(i));
      CASA_EXPECT_ARRAYEQ(expected_array, array);
      expected_array.assign(row_record.asArrayDouble("SYSVEL"));
      array.assign(mycolumns.sysvel()(i));
      CASA_EXPECT_ARRAYEQ(expected_array, array);
      Array<String> expected_transition = row_record.asArrayString(
          "TRANSITION");
      Array<String> transition = mycolumns.transition()(i);
      CASA_EXPECT_ARRAYEQ(expected_transition, transition);
    }
  }

  // verify SPECTRAL_WINDOW table
  {
    std::cout << "Verify SPECTRAL_WINDOW table" << std::endl;
    auto const mytable = myms.spectralWindow();
    TableRecord const &expected_record = reader.spw_record_;
    uInt expected_nrow = expected_record.nfields();
    ASSERT_EQ(expected_nrow, mytable.nrow());
    ROMSSpWindowColumns mycolumns(mytable);
    for (uInt i = 0; i < expected_nrow; ++i) {
      std::cout << "Verifying row " << i << std::endl;
      String key = "ROW" + String::toString(i);
      TableRecord row_record = expected_record.asRecord(key);
      EXPECT_EQ(row_record.asInt("MEAS_FREQ_REF"), mycolumns.measFreqRef()(i));
      EXPECT_EQ(row_record.asInt("NUM_CHAN"), mycolumns.numChan()(i));
      CASA_EXPECT_STREQ(row_record.asString("NAME"), mycolumns.name()(i));
      Double expected_refpix = row_record.asDouble("REFPIX");
      Double expected_refval = row_record.asDouble("REFVAL");
      Double expected_increment = row_record.asDouble("INCREMENT");
      Int expected_net_sideband = (expected_increment < 0.0) ? 1 : 0;
      EXPECT_EQ(expected_net_sideband, mycolumns.netSideband()(i));
      Vector<Double> chan_freq = mycolumns.chanFreq()(i);
      Double freq0 = chan_freq[0];
      Double chan0 = 0;
      Vector<Double> chan_width = mycolumns.chanWidth()(i);
      Double incr0 = chan_width[0];
      Double refval = freq0 + expected_refpix * incr0;
      EXPECT_EQ(expected_refval, refval);
      EXPECT_TRUE(allEQ(chan_width, expected_increment));
      Vector<Double> resolution = mycolumns.resolution()(i);
      EXPECT_TRUE(allEQ(resolution, abs(expected_increment)));
      Vector<Double> effective_bw = mycolumns.effectiveBW()(i);
      EXPECT_TRUE(allEQ(effective_bw, abs(expected_increment)));
      Double ref_freq = mycolumns.refFrequency()(i);
      EXPECT_EQ(chan_freq[0], ref_freq);
    }
  }

  // verify SYSCAL table
  {
    std::cout << "Verify SYSCAL table" << std::endl;
    auto const mytable = myms.sysCal();
    TableRecord const &expected_record = reader.syscal_record_;
    uInt expected_nrow = expected_record.nfields();
    ASSERT_EQ(expected_nrow, mytable.nrow());
    ROMSSysCalColumns mycolumns(mytable);
    for (uInt i = 0; i < expected_nrow; ++i) {
      std::cout << "Verifying row " << i << std::endl;
      String key = "ROW" + String::toString(i);
      TableRecord row_record = expected_record.asRecord(key);
      EXPECT_EQ(row_record.asDouble("TIME"), mycolumns.time()(i));
      EXPECT_EQ(row_record.asDouble("INTERVAL"), mycolumns.interval()(i));
      EXPECT_EQ(row_record.asInt("SPECTRAL_WINDOW_ID"),
          mycolumns.spectralWindowId()(i));
      EXPECT_EQ(row_record.asInt("FEED_ID"), mycolumns.feedId()(i));
      EXPECT_EQ(row_record.asInt("ANTENNA_ID"), mycolumns.antennaId()(i));
      Array<Float> expected_array = row_record.asArrayFloat("TCAL");
      Array<Float> array = mycolumns.tcal()(i);
      CASA_EXPECT_ARRAYEQ(expected_array, array);
      expected_array.assign(row_record.asArrayFloat("TSYS"));
      array.assign(mycolumns.tsys()(i));
      CASA_EXPECT_ARRAYEQ(expected_array, array);
      expected_array.assign(row_record.asArrayFloat("TCAL_SPECTRUM"));
      array.assign(mycolumns.tcalSpectrum()(i));
      CASA_EXPECT_ARRAYEQ(expected_array, array);
      expected_array.assign(row_record.asArrayFloat("TSYS_SPECTRUM"));
      array.assign(mycolumns.tsysSpectrum()(i));
      CASA_EXPECT_ARRAYEQ(expected_array, array);
    }
  }

  // verify WEATHER table
  {
    std::cout << "Verify WEATHER table" << std::endl;
    auto const mytable = myms.weather();
    TableRecord const &expected_record = reader.weather_record_;
    uInt expected_nrow = expected_record.nfields();
    ASSERT_EQ(expected_nrow, mytable.nrow());
    ROMSWeatherColumns mycolumns(mytable);
    for (uInt i = 0; i < expected_nrow; ++i) {
      std::cout << "Verifying row " << i << std::endl;
      String key = "ROW" + String::toString(i);
      TableRecord row_record = expected_record.asRecord(key);
      EXPECT_EQ(row_record.asDouble("TIME"), mycolumns.time()(i));
      EXPECT_EQ(row_record.asDouble("INTERVAL"), mycolumns.interval()(i));
      EXPECT_EQ(row_record.asInt("ANTENNA_ID"), mycolumns.antennaId()(i));
      EXPECT_FLOAT_EQ(row_record.asFloat("TEMPERATURE"),
          mycolumns.temperature()(i));
      EXPECT_FLOAT_EQ(row_record.asFloat("PRESSURE"), mycolumns.pressure()(i));
      EXPECT_FLOAT_EQ(row_record.asFloat("REL_HUMIDITY"),
          mycolumns.relHumidity()(i));
      EXPECT_FLOAT_EQ(row_record.asFloat("WIND_SPEED"),
          mycolumns.windSpeed()(i));
      EXPECT_FLOAT_EQ(row_record.asFloat("WIND_DIRECTION"),
          mycolumns.windDirection()(i));
    }
  }

  // verify HISTORY table
  {
    std::cout << "Verify HISTORY table" << std::endl;
    auto const mytable = myms.history();
    ASSERT_EQ(uInt(0), mytable.nrow());
  }

  // verify FLAG_CMD table
  {
    std::cout << "Verify FLAG_CMD table" << std::endl;
    auto const mytable = myms.flagCmd();
    ASSERT_EQ(uInt(0), mytable.nrow());
  }

  // verify DOPPLER table
  {
    std::cout << "Verify DOPPLER table" << std::endl;
    auto const mytable = myms.doppler();
    ASSERT_EQ(uInt(0), mytable.nrow());
  }

  // verify FREQ_OFFSET table
  {
    std::cout << "Verify FREQ_OFFSET table" << std::endl;
    auto const mytable = myms.freqOffset();
    ASSERT_EQ(uInt(0), mytable.nrow());
  }
}

namespace {
inline void TestKeyword(Vector<TableRecord> const &input_record,
    TableRecord const &output_record, String const &output_data_key) {
  constexpr size_t kNumInputKeys = 4;
  constexpr size_t kNumOutputKeys = 5;
  String const input_keys[kNumInputKeys] =
      { "DATA", "FLAG", "FLAG_ROW", "POLNO" };
  String output_keys[kNumOutputKeys] = { output_data_key, "FLAG", "FLAG_ROW",
      "SIGMA", "WEIGHT" };
  for (size_t i = 0; i < input_record.size(); ++i) {
    for (size_t j = 0; j < kNumInputKeys; ++j) {
      ASSERT_TRUE(input_record[i].isDefined(input_keys[j]));
    }
  }
  for (size_t j = 0; j < kNumOutputKeys; ++j) {
    ASSERT_TRUE(output_record.isDefined(output_keys[j]));
  }
}

inline void TestShape(size_t const num_pol, size_t const num_chan,
    TableRecord const &output_record, String const &output_data_key) {
  IPosition const expected_shape_matrix(2, num_pol, num_chan);
  IPosition const expected_shape_vector(1, num_pol);
  EXPECT_EQ(expected_shape_matrix, output_record.shape(output_data_key));
  EXPECT_EQ(expected_shape_matrix, output_record.shape("FLAG"));
  EXPECT_EQ(expected_shape_vector, output_record.shape("SIGMA"));
  EXPECT_EQ(expected_shape_vector, output_record.shape("WEIGHT"));
}

struct BasicPolarizationTester {
  static void Test(size_t const num_chan, Vector<uInt> const &polid_list,
      Vector<TableRecord> const &input_record,
      TableRecord const &output_record) {
    size_t num_pol = polid_list.size();
    ASSERT_LE(num_pol, 2ul);
    ASSERT_EQ(num_pol, input_record.size());
    TestKeyword(input_record, output_record, "FLOAT_DATA");

    TestShape(num_pol, num_chan, output_record, "FLOAT_DATA");

    Matrix < Float > out_data = output_record.asArrayFloat("FLOAT_DATA");
    Matrix < Bool > out_flag = output_record.asArrayBool("FLAG");
    Bool out_flag_row = output_record.asBool("FLAG_ROW");
    Vector < Float > out_sigma = output_record.asArrayFloat("SIGMA");
    Vector < Float > out_weight = output_record.asArrayFloat("WEIGHT");
    Bool net_flag_row = False;
    for (size_t i = 0; i < num_pol; ++i) {
      std::cout << "Verifying i " << i << " polid "
          << input_record[i].asuInt("POLNO") << std::endl;
      uInt polid = 0;
      if (num_pol == 2) {
        polid = input_record[i].asuInt("POLNO");
      }
      Vector < Float > data = input_record[i].asArrayFloat("DATA");
      Vector < Bool > flag = input_record[i].asArrayBool("FLAG");
      net_flag_row = net_flag_row || input_record[i].asBool("FLAG_ROW");
      std::cout << "out_data.shape() = " << out_data.shape() << std::endl;
      std::cout << "polid = " << polid << " (num_pol " << num_pol << ")"
          << std::endl;
      EXPECT_TRUE(allEQ(out_data.row(polid), data));
      EXPECT_TRUE(allEQ(out_flag.row(polid), flag));
    }
    EXPECT_EQ(net_flag_row, out_flag_row);
    EXPECT_TRUE(allEQ(out_sigma, 1.0f));
    EXPECT_TRUE(allEQ(out_weight, 1.0f));
  }
};

struct FullPolarizationTester {
  static void Test(size_t const num_chan, Vector<uInt> const &polid_list,
      Vector<TableRecord> const &input_record,
      TableRecord const &output_record) {
    std::cout << "Full polarization test" << std::endl;
    size_t num_pol = polid_list.size();
    ASSERT_LE(num_pol, 4ul);
    ASSERT_EQ(num_pol, input_record.size());

    TestKeyword(input_record, output_record, "DATA");

    TestShape(num_pol, num_chan, output_record, "DATA");

    Matrix < Float > data_real(IPosition(2, num_pol, num_chan), 0.0f);
    Matrix < Float > data_imag(data_real.shape(), 0.0f);
    Matrix < Bool > flag(data_real.shape(), False);
    Bool net_flag_row = False;
    for (size_t i = 0; i < num_pol; ++i) {
      uInt polid = polid_list[i];
      ASSERT_LT(polid, num_pol);
      Vector < Float > input_data = input_record[i].asArrayFloat("DATA");
      Vector < Bool > input_flag = input_record[i].asArrayBool("FLAG");
      net_flag_row = net_flag_row || input_record[i].asBool("FLAG_ROW");
      if (polid == 0) {
        data_real.row(0) = input_data;
        flag.row(0) = input_flag;
      } else if (polid == 1) {
        data_real.row(3) = input_data;
        flag.row(3) = input_flag;
      } else if (polid == 2) {
        data_real.row(1) = input_data;
        data_real.row(2) = input_data;
        flag.row(1) = input_flag;
      } else if (polid == 3) {
        data_imag.row(1) = input_data;
        data_imag.row(2) = -input_data;
        flag.row(2) = input_flag;
      }
    }
    flag.row(1) = flag.row(1) || flag.row(2);
    flag.row(2) = flag.row(1);

    Matrix < Complex > out_data = output_record.asArrayComplex("DATA");
    EXPECT_TRUE(allEQ(data_real, real(out_data)));
    EXPECT_TRUE(allEQ(data_imag, imag(out_data)));
    Matrix < Bool > out_flag = output_record.asArrayBool("FLAG");
//    std::cout << "expected flag: " << flag << std::endl;
//    std::cout << "output flag  : " << out_flag << std::endl;
    EXPECT_TRUE(allEQ(flag, out_flag));
    Bool out_flag_row = output_record.asBool("FLAG_ROW");
    EXPECT_EQ(net_flag_row, out_flag_row);
    Vector < Float > out_sigma = output_record.asArrayFloat("SIGMA");
    Vector < Float > out_weight = output_record.asArrayFloat("WEIGHT");
    EXPECT_TRUE(allEQ(out_sigma, 1.0f));
    EXPECT_TRUE(allEQ(out_weight, 1.0f));
  }

};

struct StokesFullPolarizationTester {
  static void Test(size_t const num_chan, Vector<uInt> const &polid_list,
      Vector<TableRecord> const &input_record,
      TableRecord const &output_record) {
    std::cout << "Stokes full polarization test" << std::endl;
    size_t num_pol = polid_list.size();
    ASSERT_LE(num_pol, 4ul);
    ASSERT_EQ(num_pol, input_record.size());

    TestKeyword(input_record, output_record, "FLOAT_DATA");

    TestShape(num_pol, num_chan, output_record, "FLOAT_DATA");

    Matrix < Float > out_data = output_record.asArrayFloat("FLOAT_DATA");
    Matrix < Bool > out_flag = output_record.asArrayBool("FLAG");
    Bool out_flag_row = output_record.asBool("FLAG_ROW");
    Vector < Float > out_sigma = output_record.asArrayFloat("SIGMA");
    Vector < Float > out_weight = output_record.asArrayFloat("WEIGHT");
    Bool net_flag_row = False;
    for (size_t i = 0; i < num_pol; ++i) {
      uInt polid = polid_list[i];
      ASSERT_LT(polid, num_pol);
      Vector < Float > data = input_record[i].asArrayFloat("DATA");
      Vector < Bool > flag = input_record[i].asArrayBool("FLAG");
      Bool flag_row = input_record[i].asBool("FLAG_ROW");
      net_flag_row = net_flag_row || flag_row;
      EXPECT_TRUE(allEQ(data, out_data.row(polid)));
      EXPECT_TRUE(allEQ(flag, out_flag.row(polid)));
    }
    EXPECT_EQ(net_flag_row, out_flag_row);
    EXPECT_TRUE(allEQ(out_sigma, 1.0f));
    EXPECT_TRUE(allEQ(out_weight, 1.0f));
  }

};

struct StandardInitializer {
  static void Initialize(size_t const num_chan, Vector<uInt> const &polid_list,
      Vector<TableRecord> &input_record, DataChunk &chunk) {
    chunk.initialize(num_chan);
    Matrix < Float > data(4, num_chan, 0.5);
    Matrix < Bool > flag(4, num_chan, False);
    Vector < Bool > flag_row(4, False);
    data(0, 16) = -0.5;
    flag(0, 16) = True;
    data(1, 8) = 1.e10;
    flag(1, 8) = True;
    data.row(2) = -1.e10;
    flag.row(2) = True;
    flag_row[2] = True;
    for (size_t i = 0; i < polid_list.size(); ++i) {
      TableRecord &input_record0 = input_record[i];
      uInt polid = polid_list[i];
      input_record0.define("POLNO", polid);
      input_record0.define("DATA", data.row(polid));
      input_record0.define("FLAG", flag.row(polid));
      input_record0.define("FLAG_ROW", flag_row(polid));
      chunk.accumulate(input_record0);
    }
  }
};

template<class Initializer = StandardInitializer,
    class Tester = BasicPolarizationTester>
void TestPolarization(String const &poltype, Vector<uInt> const &polid_list) {
  std::cout << "Test " << poltype << " polarization with pol " << polid_list
      << std::endl;

  DataChunk chunk(poltype);

  CASA_ASSERT_STREQ(poltype, chunk.getPolType());

  size_t num_chan = 32;
  Vector<TableRecord> input_record(polid_list.size());

  Initializer::Initialize(num_chan, polid_list, input_record, chunk);

  ASSERT_EQ(polid_list.size(), chunk.getNumPol());

  TableRecord output_record;
  ASSERT_TRUE(output_record.empty());

  chunk.get(output_record);

  Tester::Test(num_chan, polid_list, input_record, output_record);
}

void TestSinglePolarization(String const &poltype, uInt const polid) {
  Vector < uInt > polid_list(1, polid);
  TestPolarization(poltype, polid_list);
}

} // anonymous namespace

TEST(DataChunkTest, SinglePolarizationTest) {
  // POL 0
  uInt polid = 0;

  // Linear
  TestSinglePolarization("linear", polid);

  // Circular
  TestSinglePolarization("circular", polid);

  // Stokes
  TestSinglePolarization("stokes", polid);

  // Linpol
  TestSinglePolarization("linpol", polid);

  // POL 1
  polid = 1;

  // Linear
  TestSinglePolarization("linear", polid);

  // Circular
  TestSinglePolarization("circular", polid);

  // Stokes
  // it should cause error, see WhiteBoxTest

  // Linpol
  // it should cause error, see WhiteBoxTest
}

TEST(DataChunkTest, DualPolarizationTest) {
  // POL 0 and 1
  Vector < uInt > polid_list(2);
  polid_list[0] = 0;
  polid_list[1] = 1;

  // Linear
  TestPolarization("linear", polid_list);

  // Circular
  TestPolarization("circular", polid_list);

  // Stokes
  // it should cause unexpected behavior, see WhiteBoxTest

  // Linpol
  TestPolarization("linpol", polid_list);

  // Reverse POL order
  polid_list[0] = 1;
  polid_list[1] = 0;

  // Linear
  TestPolarization("linear", polid_list);

  // Circular
  TestPolarization("circular", polid_list);

  // Stokes
  // it should cause unexpected behavior, see WhiteBoxTest

  // Linpol
  TestPolarization("linpol", polid_list);

}

TEST(DataChunkTest, FullPolarizationTest) {
  Vector < uInt > polid_list(4);

  // Usual accumulation order
  polid_list[0] = 0;
  polid_list[1] = 1;
  polid_list[2] = 2;
  polid_list[3] = 3;

  // Linear
  TestPolarization<StandardInitializer, FullPolarizationTester>("linear",
      polid_list);

  // Circular
  TestPolarization<StandardInitializer, FullPolarizationTester>("circular",
      polid_list);

  // Stokes
  TestPolarization<StandardInitializer, StokesFullPolarizationTester>("stokes",
      polid_list);

  // Linpol
  // it should cause unexpected behavior, see WhiteBoxTest

  // Different order
  polid_list[0] = 2;
  polid_list[2] = 0;

  // Linear
  TestPolarization<StandardInitializer, FullPolarizationTester>("linear",
      polid_list);

  // Circular
  TestPolarization<StandardInitializer, FullPolarizationTester>("circular",
      polid_list);

  // Stokes
  TestPolarization<StandardInitializer, StokesFullPolarizationTester>("stokes",
      polid_list);

  // Linpol
  // it should cause unexpected behavior, see WhiteBoxTest
}

TEST(DataChunkTest, WhiteBoxTest) {
  // Invalid poltype
  EXPECT_THROW(DataChunk("notype"), AipsError);

  // Accumulate without initialization
  DataChunk chunk("linear");
  CASA_ASSERT_STREQ(String("linear"), chunk.getPolType());
  TableRecord record;
  Vector < Float > data1(1);
  Vector < Bool > flag1(1);
  record.define("POLNO", (uInt) 0);
  record.define("DATA", data1);
  record.define("FLAG", flag1);
  record.define("FLAG_ROW", False);
  ASSERT_EQ(0u, chunk.getNumPol());
  bool status = chunk.accumulate(record);
  ASSERT_TRUE(status);
  EXPECT_EQ(1u, chunk.getNumPol());
  TableRecord output_record;
  status = chunk.get(output_record);
  ASSERT_TRUE(status);
  EXPECT_EQ(IPosition(2, 1, 1),
      output_record.asArrayFloat("FLOAT_DATA").shape());

  // clear
  chunk.clear();
  EXPECT_EQ(0u, chunk.getNumPol());

  // Accumulate invalid record
  chunk.initialize(1);
  record.removeField("POLNO");
  ASSERT_EQ(0u, chunk.getNumPol());
  status = chunk.accumulate(record);
  ASSERT_FALSE(status);
  EXPECT_EQ(0u, chunk.getNumPol());

  // Accumulate different shaped data
  Vector < Float > data2(2);
  Vector < Bool > flag2(2);
  chunk.initialize(1);
  record.define("POLNO", (uInt) 0);
  record.define("DATA", data2);
  record.define("FLAG", flag2);
  record.define("FLAG_ROW", False);
  constexpr bool expected_status = false;
  status = chunk.accumulate(record);
  EXPECT_EQ(expected_status, status);

  // Shape mismatch between data and flag
  record.define("DATA", data1);
  status = chunk.accumulate(record);
  EXPECT_EQ(expected_status, status);

  // Test number of polarization
  constexpr size_t num_seq = 4ul;
  constexpr size_t num_accum = 4ul;
  uInt expected_num_pol[num_seq][num_accum] = { { 1, 2, 2, 4 }, { 1, 2, 2, 4 },
      { 0, 0, 1, 4 }, { 1, 1, 2, 4 } };
  uInt polid_order[num_seq][num_accum] = { { 0, 1, 2, 3 }, { 1, 0, 2, 3 }, { 3,
      2, 1, 0 }, { 0, 3, 1, 2 } };
  record.define("DATA", data2);
  record.define("FLAG", flag2);
  record.define("FLAG_ROW", False);
  for (size_t j = 0; j < num_seq; ++j) {
    chunk.initialize(2);
    EXPECT_EQ(0u, chunk.getNumPol());
    for (size_t i = 0; i < num_accum; ++i) {
      record.define("POLNO", polid_order[j][i]);
      status = chunk.accumulate(record);
      ASSERT_TRUE(status);
      EXPECT_EQ(expected_num_pol[j][i], chunk.getNumPol());
    }
  }

  // accumulate data to same polarization twice
  data2 = 0.0;
  chunk.initialize(2);
  record.define("POLNO", 0u);
  record.define("DATA", data2);
  chunk.accumulate(record);
  data2 = 1.0;
  record.define("DATA", data2);
  status = chunk.accumulate(record);
  ASSERT_TRUE(status);
  status = chunk.get(output_record);
  ASSERT_TRUE(status);
  Matrix < Float > output_data = output_record.asArrayFloat("FLOAT_DATA");
  EXPECT_EQ(IPosition(2, 1, 2), output_data.shape());
  EXPECT_TRUE(allEQ(output_data, 1.0f));

  // accumulate three polarization component
  chunk.initialize(2);
  data2 = 3.0;
  record.define("POLNO", 0u);
  record.define("DATA", data2);
  chunk.accumulate(record);
  record.define("POLNO", 1u);
  chunk.accumulate(record);
  record.define("POLNO", 2u);
  chunk.accumulate(record);
  EXPECT_EQ(2u, chunk.getNumPol());
  status = chunk.get(output_record);
  ASSERT_TRUE(status);
  Matrix < Float > data = output_record.asArrayFloat("FLOAT_DATA");
  EXPECT_EQ(IPosition(2, 2, 2), data.shape());
  EXPECT_TRUE(allEQ(data, 3.0f));

  // reset poltype
  EXPECT_NE(0u, chunk.getNumPol());
  chunk.resetPolType("circular");
  CASA_ASSERT_STREQ(String("circular"), chunk.getPolType());
  ASSERT_EQ(0u, chunk.getNumPol());

  // Stokes single pol 1 and dual pols 1 and 2 are invalid
  DataChunk stokes_chunk("stokes");

  stokes_chunk.initialize(2);
  data2 = 4.0;
  record.define("POLNO", 1u);
  record.define("DATA", data2);
  stokes_chunk.accumulate(record);
  EXPECT_EQ(0u, stokes_chunk.getNumPol());
  status = stokes_chunk.get(output_record);
  ASSERT_FALSE(status);
  record.define("POLNO", 0u);
  stokes_chunk.accumulate(record);
  EXPECT_EQ(1u, stokes_chunk.getNumPol());
  status = stokes_chunk.get(output_record);
  ASSERT_TRUE(status);
  Matrix < Float > data_stokes = output_record.asArrayFloat("FLOAT_DATA");
  EXPECT_EQ(IPosition(2, 1, 2), data_stokes.shape());
  EXPECT_TRUE(allEQ(data_stokes, 4.0f));

  // Test number of polarization
  uInt expected_num_pol_stokes[num_seq][num_accum] = { { 1, 1, 1, 4 }, { 0, 1,
      1, 4 }, { 0, 0, 0, 4 }, { 1, 1, 1, 4 } };
  uInt polid_order_stokes[num_seq][num_accum] = { { 0, 1, 2, 3 },
      { 1, 0, 2, 3 }, { 3, 2, 1, 0 }, { 0, 3, 1, 2 } };
  record.define("DATA", data2);
  record.define("FLAG", flag2);
  record.define("FLAG_ROW", False);
  for (size_t j = 0; j < num_seq; ++j) {
    stokes_chunk.initialize(2);
    EXPECT_EQ(0u, stokes_chunk.getNumPol());
    for (size_t i = 0; i < num_accum; ++i) {
      record.define("POLNO", polid_order_stokes[j][i]);
      status = stokes_chunk.accumulate(record);
      ASSERT_TRUE(status);
      EXPECT_EQ(expected_num_pol_stokes[j][i], stokes_chunk.getNumPol());
    }
  }

  // Linpol single pol 1 and full pols are invalid
  DataChunk linpol_chunk("linpol");

  linpol_chunk.initialize(2);
  data2 = 8.0;
  record.define("POLNO", 1u);
  record.define("DATA", data2);
  linpol_chunk.accumulate(record);
  EXPECT_EQ(0u, linpol_chunk.getNumPol());
  status = linpol_chunk.get(output_record);
  ASSERT_FALSE(status);
  record.define("POLNO", 0u);
  linpol_chunk.accumulate(record);
  record.define("POLNO", 2u);
  linpol_chunk.accumulate(record);
  record.define("POLNO", 4u);
  linpol_chunk.accumulate(record);
  EXPECT_EQ(2u, linpol_chunk.getNumPol());
  status = linpol_chunk.get(output_record);
  ASSERT_TRUE(status);
  Matrix < Float > data_linpol = output_record.asArrayFloat("FLOAT_DATA");
  EXPECT_EQ(IPosition(2, 2, 2), data_linpol.shape());
  EXPECT_TRUE(allEQ(data_linpol, 8.0f));

  // Test number of polarization
  uInt expected_num_pol_linpol[num_seq][num_accum] = { { 1, 2, 2, 2 }, { 0, 2,
      2, 2 }, { 0, 0, 0, 2 }, { 1, 1, 2, 2 } };
  uInt polid_order_linpol[num_seq][num_accum] = { { 0, 1, 2, 3 },
      { 1, 0, 2, 3 }, { 3, 2, 1, 0 }, { 0, 3, 1, 2 } };
  record.define("DATA", data2);
  record.define("FLAG", flag2);
  record.define("FLAG_ROW", False);
  for (size_t j = 0; j < num_seq; ++j) {
    linpol_chunk.initialize(2);
    EXPECT_EQ(0u, linpol_chunk.getNumPol());
    for (size_t i = 0; i < num_accum; ++i) {
      record.define("POLNO", polid_order_linpol[j][i]);
      status = linpol_chunk.accumulate(record);
      ASSERT_TRUE(status);
      EXPECT_EQ(expected_num_pol_linpol[j][i], linpol_chunk.getNumPol());
    }
  }
}

TEST(DataAccumulatorTest, WhiteBoxTest) {
  constexpr size_t num_record_keys = 10;
  constexpr const char *output_record_keys[] = { "TIME", "POL_TYPE", "INTENT",
      "SPECTRAL_WINDOW_ID", "FIELD_ID", "FEED_ID", "FLAG", "FLAG_ROW", "SIGMA",
      "WEIGHT" }; // and "DATA" or "FLOAT_DATA"
  auto IsValidOutputRecord = [&](TableRecord const &record) {
    for (size_t i = 0; i < num_record_keys; ++i) {
      ASSERT_TRUE(record.isDefined(output_record_keys[i]));
    }
    Matrix<Bool> const flag = record.asArrayBool("FLAG");
    size_t const num_pol = flag.nrow();
    String const poltype = record.asString("POL_TYPE");
    if (poltype == "linear" || poltype == "circular") {
      ASSERT_TRUE((num_pol == 1) || (num_pol == 2) || (num_pol == 4));
      if (num_pol < 4) {
        ASSERT_FALSE(record.isDefined("DATA"));
        ASSERT_TRUE(record.isDefined("FLOAT_DATA"));
      }
      else {
        ASSERT_FALSE(record.isDefined("FLOAT_DATA"));
        ASSERT_TRUE(record.isDefined("DATA"));
      }
    }
    else if (poltype == "stokes") {
      ASSERT_TRUE((num_pol == 1) || (num_pol == 4));
      ASSERT_FALSE(record.isDefined("DATA"));
      ASSERT_TRUE(record.isDefined("FLOAT_DATA"));
    }
    else if (poltype == "linpol") {
      ASSERT_TRUE((num_pol == 1) || (num_pol == 2));
      ASSERT_FALSE(record.isDefined("DATA"));
      ASSERT_TRUE(record.isDefined("FLOAT_DATA"));
    }
    else {
      FAIL();
    }
  };

  auto ClearRecord = [](TableRecord &record) {
    while (0 < record.nfields()) {
      record.removeField(0);
    }
    ASSERT_EQ(0u, record.nfields());
  };

  DataAccumulator a;

  // number of chunks must be zero at initial state
  ASSERT_EQ(0ul, a.getNumberOfChunks());
  TableRecord output_record;
  bool status = a.get(0, output_record);
  ASSERT_FALSE(status);

  TableRecord r1;
  Time t(2016, 1, 1);
  Double time = t.modifiedJulianDay() * 86400.0;
  r1.define("TIME", time);

  // accumulator should not be ready at initial state
  bool is_ready = a.queryForGet(r1);
  ASSERT_FALSE(is_ready);

  // Invalid record cannot be accumulated
  status = a.accumulate(r1);
  ASSERT_FALSE(status);
  TableRecord r2;
  time += 1.0;
  r2.define("TIME", time);
  is_ready = a.queryForGet(r2);
  ASSERT_FALSE(is_ready);

  // Accumulate one data
  Int spw_id = 0;
  Int field_id = 1;
  Int feed_id = 2;
  String intent = "ON_SOURCE";
  String poltype = "linear";
  r1.define("SPECTRAL_WINDOW_ID", spw_id);
  r1.define("FIELD_ID", field_id);
  r1.define("FEED_ID", feed_id);
  r1.define("INTENT", intent);
  r1.define("POL_TYPE", poltype);
  size_t const num_chan = 4;
//  Vector < Float > data(num_chan, 0.0f);
//  Vector < Bool > flag(num_chan, False);
  Matrix < Float > data(IPosition(2, 4, num_chan), 0.0f);
  Matrix < Bool > flag(IPosition(2, 4, num_chan), False);
  data(0, 0) = 1.e10;
  flag(0, 0) = True;
  data(1, 1) = 1.e10;
  flag(1, 1) = True;
  data(2, 2) = 1.e10;
  flag(2, 2) = True;
  data(3, 3) = 1.e10;
  flag(3, 3) = True;
  Vector < Bool > flag_row(4, False);
  flag_row[3] = True;
  r1.define("POLNO", 0);
  r1.define("DATA", data.row(0));
  r1.define("FLAG", flag.row(0));
  r1.define("FLAG_ROW", flag_row[0]);
  status = a.accumulate(r1);
  ASSERT_TRUE(status);
  EXPECT_EQ(1u, a.getNumberOfChunks());
  EXPECT_EQ(1u, a.getNumPol(0));

  // Data with different timestamp cannot be accumulated
  r2.merge(r1, TableRecord::SkipDuplicates);
  status = a.accumulate(r2);
  ASSERT_FALSE(status);
  EXPECT_EQ(1u, a.getNumberOfChunks());
  EXPECT_EQ(1u, a.getNumPol(0));

  // query
  is_ready = a.queryForGet(r1);
  ASSERT_FALSE(is_ready);

  is_ready = a.queryForGet(r2);
  ASSERT_TRUE(is_ready);

  // Get
  ClearRecord(output_record);
  CASA_ASSERT_STREQ(poltype, a.getPolType(0));
  ASSERT_EQ(1u, a.getNumPol(0));
  status = a.get(0, output_record);
  ASSERT_TRUE(status);
  IsValidOutputRecord(output_record);
  Matrix < Float > output_data = output_record.asArrayFloat("FLOAT_DATA");
  Matrix < Bool > output_flag = output_record.asArrayBool("FLAG");
  IPosition const expected_shape1(2, 1, num_chan);
  EXPECT_EQ(expected_shape1, output_data.shape());
  EXPECT_EQ(expected_shape1, output_flag.shape());
  EXPECT_TRUE(allEQ(output_data.row(0), data.row(0)));
  EXPECT_TRUE(allEQ(output_flag.row(0), flag.row(0)));
  EXPECT_EQ(flag_row[0], output_record.asBool("FLAG_ROW"));
//  for (size_t i = 0; i < a.getNumberOfChunks(); ++i) {
//    CASA_ASSERT_STREQ(poltype, a.getPolType(i));
//    ASSERT_EQ(1u, a.getNumPol(i));
//    status = a.get(i, output_record);
//    ASSERT_TRUE(status);
//    IsValidOutputRecord(output_record);
//    Matrix < Float > output_data = output_record.asArrayFloat("FLOAT_DATA");
//    Matrix < Bool > output_flag = output_record.asArrayBool("FLAG");
//    IPosition const expected_shape(2, 1, num_chan);
//    EXPECT_EQ(expected_shape, output_data.shape());
//    EXPECT_EQ(expected_shape, output_flag.shape());
//    EXPECT_TRUE(allEQ(output_data.row(0), data.row(i)));
//    EXPECT_TRUE(allEQ(output_flag.row(0), flag.row(i)));
//  }

  // Accumulate more data with same meta data but different polno
  r1.define("POLNO", 1);
  r1.define("DATA", data.row(1));
  r1.define("FLAG", flag.row(1));
  r1.define("FLAG_ROW", flag_row[1]);
  status = a.accumulate(r1);
  ASSERT_TRUE(status);
  EXPECT_EQ(1u, a.getNumberOfChunks());

  // Get
  ClearRecord(output_record);
  CASA_ASSERT_STREQ(poltype, a.getPolType(0));
  ASSERT_EQ(2u, a.getNumPol(0));
  status = a.get(0, output_record);
  ASSERT_TRUE(status);
  IsValidOutputRecord(output_record);
  output_data.assign(output_record.asArrayFloat("FLOAT_DATA"));
  output_flag.assign(output_record.asArrayBool("FLAG"));
  IPosition const expected_shape2(2, 2, num_chan);
  EXPECT_EQ(expected_shape2, output_data.shape());
  EXPECT_EQ(expected_shape2, output_flag.shape());
  EXPECT_TRUE(allEQ(output_data.row(0), data.row(0)));
  EXPECT_TRUE(allEQ(output_flag.row(0), flag.row(0)));
  EXPECT_TRUE(allEQ(output_data.row(1), data.row(1)));
  EXPECT_TRUE(allEQ(output_flag.row(1), flag.row(1)));
  EXPECT_EQ(flag_row[0] || flag_row[1], output_record.asBool("FLAG_ROW"));

  // Accumulate cross-pol data with same meta data
  r1.define("POLNO", 2);
  r1.define("DATA", data.row(2));
  r1.define("FLAG", flag.row(2));
  r1.define("FLAG_ROW", flag_row[2]);
  status = a.accumulate(r1);
  ASSERT_TRUE(status);
  EXPECT_EQ(1u, a.getNumberOfChunks());
  r1.define("POLNO", 3);
  r1.define("DATA", data.row(3));
  r1.define("FLAG", flag.row(3));
  r1.define("FLAG_ROW", flag_row[3]);
  status = a.accumulate(r1);
  ASSERT_TRUE(status);
  EXPECT_EQ(1u, a.getNumberOfChunks());

  // Get
  ClearRecord(output_record);
  CASA_ASSERT_STREQ(poltype, a.getPolType(0));
  ASSERT_EQ(4u, a.getNumPol(0));
  status = a.get(0, output_record);
  ASSERT_TRUE(status);
  IsValidOutputRecord(output_record);
  Matrix < Complex > output_cdata = output_record.asArrayComplex("DATA");
  output_flag.assign(output_record.asArrayBool("FLAG"));
  IPosition const expected_shape4(2, 4, num_chan);
  EXPECT_EQ(expected_shape4, output_cdata.shape());
  EXPECT_EQ(expected_shape4, output_flag.shape());
  EXPECT_TRUE(allEQ(real(output_cdata.row(0)), data.row(0)));
  EXPECT_TRUE(allEQ(imag(output_cdata.row(0)), 0.0f));
  EXPECT_TRUE(allEQ(output_flag.row(0), flag.row(0)));
  EXPECT_TRUE(allEQ(real(output_cdata.row(1)), data.row(2)));
  EXPECT_TRUE(allEQ(imag(output_cdata.row(1)), data.row(3)));
  EXPECT_TRUE(allEQ(output_flag.row(1), (flag.row(2) || flag.row(3))));
  EXPECT_TRUE(allEQ(conj(output_cdata.row(1)), output_cdata.row(2)));
  EXPECT_TRUE(allEQ(output_flag.row(1), output_flag.row(2)));
  EXPECT_TRUE(allEQ(real(output_cdata.row(3)), data.row(1)));
  EXPECT_TRUE(allEQ(imag(output_cdata.row(3)), 0.0f));
  EXPECT_TRUE(allEQ(output_flag.row(3), flag.row(1)));
  EXPECT_EQ(anyTrue(flag_row), output_record.asBool("FLAG_ROW"));

  // Accumulate data with another meta data
  String intent2 = "OFF_SOURCE";
  String poltype2 = "circular";
  r1.define("POLNO", 0);
  r1.define("SPECTRA_WINDOW_ID", spw_id + 1);
  r1.define("FIELD_ID", field_id + 1);
  r1.define("FEED_ID", feed_id + 1);
  r1.define("INTENT", intent2);
  r1.define("POL_TYPE", poltype2);
  Vector < Float > data2(8, -1.0f);
  Vector < Bool > flag2(8, False);
  r1.define("DATA", data2);
  r1.define("FLAG", flag2);
  status = a.accumulate(r1);
  ASSERT_TRUE(status);
  EXPECT_EQ(2u, a.getNumberOfChunks());
  EXPECT_EQ(4u, a.getNumPol(0));
  EXPECT_EQ(1u, a.getNumPol(1));
  CASA_EXPECT_STREQ(poltype, a.getPolType(0));
  CASA_EXPECT_STREQ(poltype2, a.getPolType(1));
  ClearRecord(output_record);
  status = a.get(0, output_record);
  ASSERT_TRUE(status);
  IsValidOutputRecord(output_record);
  Matrix < Complex > output_cdata2 = output_record.asArrayComplex("DATA");
  EXPECT_EQ(output_cdata.shape(), output_cdata2.shape());
  EXPECT_TRUE(allEQ(output_cdata, output_cdata2));
  ClearRecord(output_record);
  status = a.get(1, output_record);
  ASSERT_TRUE(status);
  IsValidOutputRecord(output_record);
  Matrix < Float > output_data2 = output_record.asArrayFloat("FLOAT_DATA");
  EXPECT_EQ(IPosition(2, 1, 8), output_data2.shape());
  EXPECT_TRUE(allEQ(output_data2.row(0), data2));
  Matrix < Bool > output_flag2 = output_record.asArrayBool("FLAG");
  EXPECT_EQ(output_data2.shape(), output_flag2.shape());
  EXPECT_TRUE(allEQ(output_flag2.row(0), flag2));

  // Accumulate another
  r1.define("POLNO", 1);
  data2 *= 2.0f;
  flag2 = True;
  r1.define("DATA", data2);
  r1.define("FLAG", flag2);
  status = a.accumulate(r1);
  ASSERT_TRUE(status);
  EXPECT_EQ(2u, a.getNumberOfChunks());
  EXPECT_EQ(2u, a.getNumPol(1));
  ClearRecord(output_record);
  status = a.get(1, output_record);
  IsValidOutputRecord(output_record);
  output_data2.assign(output_record.asArrayFloat("FLOAT_DATA"));
  output_flag2.assign(output_record.asArrayBool("FLAG"));
  EXPECT_EQ(IPosition(2, 2, 8), output_data2.shape());
  EXPECT_TRUE(allEQ(data2, output_data2.row(1)));
  EXPECT_EQ(output_data2.shape(), output_flag2.shape());
  EXPECT_TRUE(allEQ(flag2, output_flag2.row(1)));

  // clear
  a.clear();
  EXPECT_EQ(2u, a.getNumberOfChunks());
  EXPECT_EQ(0u, a.getNumberOfActiveChunks());
  EXPECT_FALSE(a.queryForGet(r1));

  // reuse underlying DataChunk object
}

int main(int nArgs, char * args[]) {
  ::testing::InitGoogleTest(&nArgs, args);
  std::cout << "SingleDishMSFiller test " << std::endl;
  return RUN_ALL_TESTS();
}