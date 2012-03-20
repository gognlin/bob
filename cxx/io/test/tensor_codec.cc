/**
 * @file cxx/io/test/tensor_codec.cc
 * @date Wed Jun 22 17:50:08 2011 +0200
 * @author Andre Anjos <andre.anjos@idiap.ch>
 *
 * @brief ImageArrayCodec tests
 *
 * Copyright (C) 2011-2012 Idiap Research Institute, Martigny, Switzerland
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TensorArrayCodec Tests
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_array.hpp>

#include <blitz/array.h>
#include "core/logging.h"
#include "io/Array.h"

struct T {
  blitz::Array<int8_t,2> a, b;

  T() {
    a.resize(6,4);
    a = 1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24;
    b.resize(3,4);
    b = 0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11;
  }

  ~T() { }

};

/**
 * @brief Generates a unique temporary filename, closes the file and return
 * its name. 
 *
 * Yes, I know this is not 100% secure...
 */
std::string temp_file(const std::string& ext) {
  boost::filesystem::path tpl = bob::core::tmpdir();
  std::string filename("bobtest_core_tensorformatXXXXXX");
  filename.append(ext);
  tpl /= filename;
  boost::shared_array<char> char_tpl(new char[tpl.file_string().size()+1]);
  strcpy(char_tpl.get(), tpl.file_string().c_str());
  int fd = mkstemps(char_tpl.get(), ext.size());
  close(fd);
  boost::filesystem::remove(char_tpl.get());
  std::string res = char_tpl.get();
  return res;
}

template<typename T, typename U> 
void check_equal(const blitz::Array<T,2>& a, const blitz::Array<U,2>& b) 
{
  BOOST_REQUIRE_EQUAL(a.extent(0), b.extent(0));
  BOOST_REQUIRE_EQUAL(a.extent(1), b.extent(1));
  for (int i=0; i<a.extent(0); ++i) {
    for (int j=0; j<a.extent(1); ++j) {
      BOOST_CHECK_EQUAL(a(i,j), bob::core::cast<T>(b(i,j)));
    }
  }
}

BOOST_FIXTURE_TEST_SUITE( test_setup, T )

BOOST_AUTO_TEST_CASE( tensor_2d )
{
  // Prepare io Array from blitz array
  bob::io::Array db_a(a);
  BOOST_CHECK_EQUAL(db_a.getNDim(), a.dimensions());
  BOOST_CHECK_EQUAL(db_a.getElementType(), bob::core::array::t_int8);
  BOOST_CHECK_EQUAL(db_a.isLoaded(), true);
  BOOST_CHECK_EQUAL(db_a.getFilename().size(), 0);
  BOOST_CHECK_EQUAL(db_a.getCodec().use_count(), 0);
  for(size_t i=0; i<db_a.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_a.getShape()[i], a.extent(i));
  check_equal( db_a.get<int8_t,2>(), a );

  // Save to .tensor
  std::string filename = temp_file(".tensor");
  db_a.save(filename);

  // Readd .tensor
  bob::io::Array db_a_read(filename);
  check_equal( db_a_read.get<int8_t,2>(), a);

  // Clean-up
  boost::filesystem::remove(filename);
}

BOOST_AUTO_TEST_CASE( tensor_2d_read_T5alpha )
{
  // Get path to the XML Schema definition
  char *testdata_cpath = getenv("BOB_TESTDATA_DIR");
  if( !testdata_cpath || !strcmp( testdata_cpath, "") ) {
    throw std::runtime_error("Environment variable $BOB_TESTDATA_DIR is not set. Have you setup your working environment correctly?");
  }
  boost::filesystem::path testdata_path( testdata_cpath);
  testdata_path /= "tensor_char.tensor";

  // Prepare io Array from Tensor file saved with torch5spro alpha
  bob::io::Array db_b(testdata_path.string());
  BOOST_CHECK_EQUAL(db_b.getNDim(), b.dimensions());
  BOOST_CHECK_EQUAL(db_b.getElementType(), bob::core::array::t_int8);
  BOOST_CHECK_EQUAL(db_b.isLoaded(), false);
  for(size_t i=0; i<db_b.getNDim(); ++i)
    BOOST_CHECK_EQUAL(db_b.getShape()[i], b.extent(i));
  check_equal( db_b.get<int8_t,2>(), b );
}

BOOST_AUTO_TEST_SUITE_END()
