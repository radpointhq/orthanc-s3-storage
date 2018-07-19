#include <iostream>
#include "gtest/gtest.h"

#include "MemStreamBuf.hpp"

namespace {

TEST(MemStreamBuf, NullConstructor) {
    Stream::MemStreamBuf buf;

    EXPECT_EQ(buf.get(), nullptr);
    EXPECT_EQ(buf.size(), 0);
    EXPECT_TRUE(buf.isOwning());
}

TEST(MemStreamBuf, Constructor) {
    size_t size = 1000;
    char* p = new char[size];
    Stream::MemStreamBuf buf(p, size);

    EXPECT_EQ(buf.get(), p);
    EXPECT_EQ(buf.allocsize(), size);
    EXPECT_FALSE(buf.isOwning());
}

TEST(MemStreamBuf, StringConstructor) {
    std::string s("Test String.");

    Stream::MemStreamBuf buf(&s[0], s.size());
    EXPECT_EQ(*buf.get(), s[0]);
    EXPECT_EQ(buf.size(), 0); //?
    EXPECT_EQ(buf.allocsize(), s.size());
    EXPECT_FALSE(buf.isOwning());
}

TEST(MemStreamBuf, StreamTest1) {
    size_t size = 100;

    void* p = static_cast<void*>(new char[size]);
    Stream::MemStreamBuf buf(static_cast<char*>(p), size);

    std::ostream os(&buf);

    os << "123";

    EXPECT_EQ(std::memcmp(buf.get(), "123", 3), 0);
    EXPECT_EQ(buf.str().size(), 3);
    EXPECT_EQ(buf.size(), 3); //?
    EXPECT_EQ(buf.allocsize(), size); //?
}

TEST(MemStreamBuf, StreamTest2) {
    Stream::MemStreamBuf buf;

    std::ostream os(&buf);

    const char* s= "123";
    os << s;

    EXPECT_EQ(0, std::memcmp(buf.get(), s, 3));
    EXPECT_EQ(buf.str().size(), std::strlen(s));
    EXPECT_EQ(buf.size(), std::strlen(s));
}

TEST(MemStreamBuf, StreamTest3) {
    Stream::MemStreamBuf buf;

    std::stringstream oss;
    oss << "123";

    std::ostream os(&buf);
    os << oss.rdbuf();

    EXPECT_EQ(std::memcmp(buf.get(), oss.str().c_str(), 3), 0);
    EXPECT_EQ(buf.str().size(), oss.str().size());
    EXPECT_EQ(buf.size(), oss.str().size());
}

TEST(MemStreamBuf, StreamTestSeek1) {
    Stream::MemStreamBuf buf;

    std::ostream os(&buf);

    std::string s1("123");
    std::string s2("456");
    std::string out("123456");
    os << s1;
    os.seekp(3, std::ios_base::beg);
    os << s2;

    EXPECT_EQ(0, std::memcmp(buf.get(), out.c_str(), out.size()));
    EXPECT_EQ(buf.size(), out.size()); //?
    EXPECT_EQ(os.tellp(), out.size()); //?
}

TEST(MemStreamBuf, StreamTestSeek2) {
    Stream::MemStreamBuf buf;

    std::ostream os(&buf);

    std::string s1("12345");
    std::string s2("56789");
    std::string s3("abcde");
    std::string out("1234abcde56789");
    os << s1;
    os.seekp(9, std::ios_base::beg);
    os << s2;
    os.seekp(4, std::ios_base::beg);
    os << s3;

    EXPECT_TRUE(os.good());

    os.seekp(0, std::ios_base::end);
    EXPECT_EQ(buf.size(), out.size());
    EXPECT_EQ(buf.allocsize(), out.size());

    EXPECT_EQ(buf.str(), out);
    EXPECT_EQ(buf.str().size(), out.size());
}

TEST(MemStreamBuf, StreamTestSeek3) {
    Stream::MemStreamBuf buf;

    std::ostream os(&buf);
    std::ostringstream stdos;

    size_t size = 10 * 1024 * 1024;
    std::string s1(size, 'a');
    std::string s2(size, 'b');
    std::string s3(size, 'c');

    //part 3
    os.seekp(2 * size, std::ios_base::beg);
    os << s3;
    //part 2
    os.seekp(size, std::ios_base::beg);
    os << s2;
    //and part 1
    os.seekp(0, std::ios_base::beg);
    os << s1;

    //mock it
    stdos << s1 << s2 << s3;

    EXPECT_TRUE(os.good());

    os.seekp(0, std::ios_base::end);
    EXPECT_EQ(std::memcmp(buf.get(), stdos.rdbuf()->str().c_str(), 3*size), 0);
    EXPECT_EQ(buf.allocsize(), 3*size); //?
    EXPECT_EQ(buf.size(), 3*size); //?
}

TEST(MemStreamBuf, StreamTestSeek3a) {
    Stream::MemStreamBuf buf;

    std::ostream os(&buf);
    std::ios_base::iostate exceptionMask = os.exceptions() | std::ios::failbit;
    os.exceptions(exceptionMask);

    std::stringstream stdos;

    std::size_t size = 10;
    std::string s1(size, 'f');
    std::ostringstream oss1(s1);
    std::string s2(size, 'g');
    std::ostringstream oss2(s2);
    std::string s3(size, 'h');
    std::ostringstream oss3(s3);

    //part 3
    os.seekp(2 * size, std::ios_base::beg);
    std::cout << "1: "
              << "size: " << buf.size() << ", "
              << "p: " << os.tellp()
              << '\n';
    os << oss1.rdbuf();
    std::cout << "1a: "
              << "size: " << buf.size() << ", "
              << "p: " << os.tellp() << ", "
              << '\n';
    //part 2
    os.seekp(size, std::ios_base::beg);
    std::cout << "2: "
              << "size: " << buf.size() << ", "
              << "p: " << os.tellp()
              << '\n';
    os << oss2.rdbuf();
    std::cout << "2a: "
              << "size: " << buf.size() << ", "
              << "p: " << os.tellp()
              << '\n';
    //and part 1
    os.seekp(0, std::ios_base::beg);
    os << oss3.rdbuf();

    //mock it
    stdos << s1 << s2 << s3;

    EXPECT_TRUE(os.good());

    os.seekp(0, std::ios_base::end);
    EXPECT_EQ(std::memcmp(buf.get(), stdos.rdbuf()->str().c_str(), 3*size), 0);
    EXPECT_EQ(buf.allocsize(), 3*size); //?
    EXPECT_EQ(buf.size(), 3*size); //?
}

} //namespace
