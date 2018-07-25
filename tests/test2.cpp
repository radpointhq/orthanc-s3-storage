#include <iostream>
#include "gtest/gtest.h"

#include "MemStreamBuf.hpp"
#include "Timer.hpp"

namespace {

class BufTimerTest : public testing::Test {
public:
    BufTimerTest():
        _os(&_buf),
        _chunks(6),
        _chunk_size(5 * 1024 * 1024)
    {
        _samples.resize(_chunks);

        for (unsigned int i = 0; i< _chunks; i++) {
            _samples[i] = std::string(_chunk_size, 'a' + i );
        }
    }

protected:
    Stream::MemStreamBuf _buf;
    std::ostream _os;

    std::ostringstream _oss;

    std::vector<std::string> _samples;
    const size_t _chunks;
    const size_t _chunk_size;
};

TEST_F(BufTimerTest, SimpleTest) {
    _os << "123";
    EXPECT_TRUE(_os.good());
}

TEST_F(BufTimerTest, ChunkTest1abc) {

    EXPECT_TRUE(_os.good());

    OrthancPlugins::Stopwatch stopwatch1;

    for (size_t i = 0; i < _chunks; i++) {
        std::stringstream ss(_samples[i]);
        _os << ss.rdbuf();
    }
    std::cout << "Stopwatch: " << stopwatch1.elapsed<std::chrono::milliseconds>() << " [ms]\n";

    //mock it
    OrthancPlugins::Stopwatch stopwatch2;

    for (size_t i = 0; i < _chunks; i++) {
        std::stringstream ss(_samples[i]);
        _oss << ss.rdbuf();
    }
    std::cout << "Stopwatch reference time: " << stopwatch2.elapsed<std::chrono::milliseconds>() << " [ms]\n";

    EXPECT_TRUE(_os.good());

    _os.seekp(0, std::ios_base::end);
    EXPECT_EQ(std::memcmp(_buf.get(), _oss.rdbuf()->str().c_str(), _chunks*_chunk_size), 0);
    EXPECT_EQ(_buf.size(), _chunks*_chunk_size); //?
    EXPECT_GE(_buf.allocsize(), _chunks*_chunk_size); //?
}

TEST_F(BufTimerTest, ChunkTest1cba) {

    EXPECT_EQ(_buf.size(), 0);

    OrthancPlugins::Stopwatch stopwatch1;
    for (size_t i = _chunks; i > 0; i--) {
        _os.seekp ((i-1) * _chunk_size, std::ios::beg);

        std::stringstream ss(_samples[i-1]);
        _os << ss.rdbuf();
    }
    std::cout << "Stopwatch: " << stopwatch1.elapsed<std::chrono::milliseconds>() << " [ms]\n";

    //mock it
    OrthancPlugins::Stopwatch stopwatch2;
    for (size_t i = 0; i < _chunks; i++) {
        std::stringstream ss(_samples[i]);
        _oss << ss.rdbuf();
    }
    std::cout << "Stopwatch reference time: " << stopwatch2.elapsed<std::chrono::milliseconds>() << " [ms]\n";

    EXPECT_TRUE(_os.good());

    _os.seekp(0, std::ios_base::end);
    EXPECT_EQ(std::memcmp(_buf.get(), _oss.rdbuf()->str().c_str(), _chunks*_chunk_size), 0);
    EXPECT_GE(_buf.allocsize(), _chunks*_chunk_size); //?
    EXPECT_EQ(_buf.size(), _chunks*_chunk_size); //?
}

} //namespace
