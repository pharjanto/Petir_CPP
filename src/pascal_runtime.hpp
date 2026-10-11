#pragma once
#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace pas {
// Delphi arrays retain their declared lower bound, including negative indices.
template<class T, int Low, int High> struct Array {
    T values[High - Low + 1];
    T& operator[](std::int64_t index) {
#ifdef PETIR_CHECK_BOUNDS
        if (index < Low || index > High) throw std::out_of_range("Pascal array index " + std::to_string(index) + " outside " + std::to_string(Low) + ".." + std::to_string(High));
#endif
        return values[index-Low];
    }
    const T& operator[](std::int64_t index) const {
#ifdef PETIR_CHECK_BOUNDS
        if (index < Low || index > High) throw std::out_of_range("Pascal array index " + std::to_string(index) + " outside " + std::to_string(Low) + ".." + std::to_string(High));
#endif
        return values[index-Low];
    }
};
template<class T> struct Address {
    T* pointer;
    template<class U> operator U*() const { return reinterpret_cast<U*>(pointer); }
};
template<class T> Address<T> address(T& variable) { return {&variable}; }
template<class T> void fillChar(T& target, std::size_t byteCount, int byteValue) { std::memset(&target,byteValue,byteCount); }
template<class T> void setLength(std::vector<T>& vector, std::size_t newSize) { vector.resize(newSize); }
template<class A,class B> constexpr std::int32_t add(const A& left,const B& right) {return static_cast<std::int32_t>(std::uint32_t(left)+std::uint32_t(right));}
template<class A,class B> constexpr std::int32_t sub(const A& left,const B& right) {return static_cast<std::int32_t>(std::uint32_t(left)-std::uint32_t(right));}
template<class A,class B> constexpr std::int32_t mul(const A& left,const B& right) {return static_cast<std::int32_t>(std::uint32_t(left)*std::uint32_t(right));}
template<class A> constexpr std::int32_t negate(const A& value) {return static_cast<std::int32_t>(0u-std::uint32_t(value));}
template<class T,class D> T increment(T value,D delta) {
    if constexpr(std::is_pointer_v<T>)return value+delta;
    else {using U=std::make_unsigned_t<T>;return static_cast<T>(static_cast<U>(value)+static_cast<U>(delta));}
}
template<class T,class D> T decrement(T value,D delta) {
    if constexpr(std::is_pointer_v<T>)return value-delta;
    else {using U=std::make_unsigned_t<T>;return static_cast<T>(static_cast<U>(value)-static_cast<U>(delta));}
}
template<class T, class S> constexpr auto shiftLeft(T value, S shiftCount) {
    using R=std::conditional_t<(sizeof(T)<4),std::int32_t,T>;
    using U=std::make_unsigned_t<R>;
    return static_cast<R>(static_cast<U>(value) << (static_cast<unsigned>(shiftCount)&(sizeof(R)*8-1)));
}
template<class T, class S> constexpr auto shiftRight(T value, S shiftCount) {
    using R=std::conditional_t<(sizeof(T)<4),std::int32_t,T>;
    using U=std::make_unsigned_t<R>;
    return static_cast<R>(static_cast<U>(value) >> (static_cast<unsigned>(shiftCount)&(sizeof(R)*8-1)));
}

// Delphi's string subscripts start at one. Reading one beyond the end yields
// the terminator; malformed notation is rejected by the protocol adapter.
struct String {
    std::string text;
    String()=default;
    String(const char* source):text(source){}
    String(std::string source):text(std::move(source)){}
    String(char character):text(1,character){}
    char operator[](int index) const { return index>0 && std::size_t(index)<=text.size() ? text[index-1] : '\0'; }
    char& operator[](int index) {
        if(index<=0 || std::size_t(index)>text.size()) throw std::out_of_range("Pascal string index");
        return text[index-1];
    }
    friend String operator+(const String& left,const String& right) { return left.text+right.text; }
    friend bool operator==(const String& left,const String& right) { return left.text==right.text; }
    friend bool operator!=(const String& left,const String& right) { return !(left==right); }
};
inline int length(const String& text) { return static_cast<int>(text.text.size()); }
inline int charCode(char character) { return static_cast<unsigned char>(character); }
inline char charFromCode(int code) { return static_cast<char>(code); }
inline String intToStr(std::int64_t value) { return std::to_string(value); }
inline int strToInt(const String& text) { return std::stoi(text.text); }
inline String copy(const String& text,int start,int count) {
    if(start<1 || count<=0 || std::size_t(start)>text.text.size()) return {};
    return text.text.substr(start-1,count);
}
inline void deleteText(String& text,int start,int count) { if(start>=1 && std::size_t(start)<=text.text.size())text.text.erase(start-1,count); }
inline String upperCase(String text) { for(char& character:text.text)character=static_cast<char>(std::toupper(static_cast<unsigned char>(character))); return text; }
inline int findPosition(const String& needle,const String& haystack) { auto found=haystack.text.find(needle.text);return found==std::string::npos?0:static_cast<int>(found+1); }
inline void parseInt(const String& text,int& result,int& errorCode) {
    try {std::size_t parsedLength;result=std::stoi(text.text,&parsedLength);errorCode=parsedLength==text.text.size()?0:static_cast<int>(parsedLength+1);}
    catch(...){result=0;errorCode=1;}
}
template<class T> auto abs(T value) { return std::abs(value); }
inline int round(double value) { return static_cast<int>(std::nearbyint(value)); }
inline int floor(double value) { return static_cast<int>(std::floor(value)); }
inline double sqrt(double value) { return std::sqrt(value); }

// Delphi System.Random: 32-bit LCG and high word of the 64-bit range product.
inline std::uint32_t randomSeed=0;
inline int random(int range) {
    randomSeed=randomSeed*0x08088405u+1u;
    return static_cast<int>((std::uint64_t(randomSeed)*std::uint32_t(range))>>32);
}
inline std::uint64_t delphiHashRandom() {
    const std::uint64_t smallFactor=random(65536)+8192;
    const std::uint64_t firstLargeFactor=random(214748367)+1;
    const std::uint64_t secondLargeFactor=random(214748367)+1;
    return (secondLargeFactor*firstLargeFactor*smallFactor)<<1;
}
inline std::int32_t tickCount() {
    return static_cast<std::int32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
}
inline std::function<void(const String&)> output;
inline void sendWinboard(const String& text) { if(output)output(text); }
inline void showMessage(const String& text) { std::cerr<<text.text<<'\n'; }
struct StringList {};
struct Form { String defaultDirectory="books"; String caption; };
inline Form mainForm;

// Only book and legacy PGN routines use Pascal file operations.
struct File { std::filesystem::path name; std::fstream stream; };
inline int lastIoResult=0;
inline std::filesystem::path nativePath(const String& text) {
    std::string path=text.text;for(char& character:path)if(character=='\\')character='/';return path;
}
inline bool fileExists(const String& fileName) { return std::filesystem::exists(nativePath(fileName)); }
inline void assignFile(File& file,const String& fileName) { file.name=nativePath(fileName); }
inline void reset(File& file,int=0) {
    file.stream.open(file.name,std::ios::in|std::ios::binary);lastIoResult=file.stream?0:1;
    if(lastIoResult)throw std::runtime_error("Cannot open "+file.name.string());
}
inline void rewrite(File& file) {
    file.stream.open(file.name,std::ios::out|std::ios::binary|std::ios::trunc);lastIoResult=file.stream?0:1;
    if(lastIoResult)throw std::runtime_error("Cannot write "+file.name.string());
}
inline int ioResult() {int result=lastIoResult;lastIoResult=0;return result;}
template<class T> void blockRead(File& file,T& buffer,std::size_t byteCount,int& bytesRead) {file.stream.read(reinterpret_cast<char*>(&buffer),byteCount);bytesRead=static_cast<int>(file.stream.gcount());}
template<class T> void write(File& file,const T& value,std::size_t byteCount) {file.stream.write(reinterpret_cast<const char*>(&value),byteCount);if(!file.stream)throw std::runtime_error("Book write failed: "+file.name.string());}
inline void closeFile(File& file) { file.stream.close(); }
inline bool endOfFile(File& file) { return file.stream.peek()==std::char_traits<char>::eof(); }
inline void readLine(File& file,String& line) { std::getline(file.stream,line.text);if(!line.text.empty() && line.text.back()=='\r')line.text.pop_back(); }
}
