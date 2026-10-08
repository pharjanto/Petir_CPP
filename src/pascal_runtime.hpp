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
    T& operator[](std::int64_t i) {
#ifdef PETIR_CHECK_BOUNDS
        if (i < Low || i > High) throw std::out_of_range("Pascal array index " + std::to_string(i) + " outside " + std::to_string(Low) + ".." + std::to_string(High));
#endif
        return values[i-Low];
    }
    const T& operator[](std::int64_t i) const {
#ifdef PETIR_CHECK_BOUNDS
        if (i < Low || i > High) throw std::out_of_range("Pascal array index " + std::to_string(i) + " outside " + std::to_string(Low) + ".." + std::to_string(High));
#endif
        return values[i-Low];
    }
};
template<class T> struct Address {
    T* pointer;
    template<class U> operator U*() const { return reinterpret_cast<U*>(pointer); }
};
template<class T> Address<T> address(T& v) { return {&v}; }
template<class T> void fillchar(T& v, std::size_t count, int value) { std::memset(&v,value,count); }
template<class T> void setlength(std::vector<T>& v, std::size_t n) { v.resize(n); }
template<class A,class B> constexpr std::int32_t add(const A& a,const B& b) {return static_cast<std::int32_t>(std::uint32_t(a)+std::uint32_t(b));}
template<class A,class B> constexpr std::int32_t sub(const A& a,const B& b) {return static_cast<std::int32_t>(std::uint32_t(a)-std::uint32_t(b));}
template<class A,class B> constexpr std::int32_t mul(const A& a,const B& b) {return static_cast<std::int32_t>(std::uint32_t(a)*std::uint32_t(b));}
template<class A> constexpr std::int32_t negate(const A& a) {return static_cast<std::int32_t>(0u-std::uint32_t(a));}
template<class T,class D> T increment(T v,D d) {
    if constexpr(std::is_pointer_v<T>)return v+d;
    else {using U=std::make_unsigned_t<T>;return static_cast<T>(static_cast<U>(v)+static_cast<U>(d));}
}
template<class T,class D> T decrement(T v,D d) {
    if constexpr(std::is_pointer_v<T>)return v-d;
    else {using U=std::make_unsigned_t<T>;return static_cast<T>(static_cast<U>(v)-static_cast<U>(d));}
}
template<class T, class S> constexpr auto shl(T v, S n) {
    using R=std::conditional_t<(sizeof(T)<4),std::int32_t,T>;
    using U=std::make_unsigned_t<R>;
    return static_cast<R>(static_cast<U>(v) << (static_cast<unsigned>(n)&(sizeof(R)*8-1)));
}
template<class T, class S> constexpr auto shr(T v, S n) {
    using R=std::conditional_t<(sizeof(T)<4),std::int32_t,T>;
    using U=std::make_unsigned_t<R>;
    return static_cast<R>(static_cast<U>(v) >> (static_cast<unsigned>(n)&(sizeof(R)*8-1)));
}

// Delphi's string subscripts start at one. Reading one beyond the end yields
// the terminator; malformed notation is rejected by the protocol adapter.
struct String {
    std::string value;
    String()=default;
    String(const char* s):value(s){}
    String(std::string s):value(std::move(s)){}
    String(char c):value(1,c){}
    char operator[](int i) const { return i>0 && std::size_t(i)<=value.size() ? value[i-1] : '\0'; }
    char& operator[](int i) {
        if(i<=0 || std::size_t(i)>value.size()) throw std::out_of_range("Pascal string index");
        return value[i-1];
    }
    friend String operator+(const String& a,const String& b) { return a.value+b.value; }
    friend bool operator==(const String& a,const String& b) { return a.value==b.value; }
    friend bool operator!=(const String& a,const String& b) { return !(a==b); }
};
inline int length(const String& s) { return static_cast<int>(s.value.size()); }
inline int ord(char c) { return static_cast<unsigned char>(c); }
inline char chr(int c) { return static_cast<char>(c); }
inline String inttostr(std::int64_t v) { return std::to_string(v); }
inline int strtoint(const String& s) { return std::stoi(s.value); }
inline String copy(const String& s,int start,int n) {
    if(start<1 || n<=0 || std::size_t(start)>s.value.size()) return {};
    return s.value.substr(start-1,n);
}
inline void p_delete(String& s,int start,int n) { if(start>=1 && std::size_t(start)<=s.value.size())s.value.erase(start-1,n); }
inline String uppercase(String s) { for(char& c:s.value)c=static_cast<char>(std::toupper(static_cast<unsigned char>(c))); return s; }
inline int pos(const String& needle,const String& s) { auto n=s.value.find(needle.value);return n==std::string::npos?0:static_cast<int>(n+1); }
inline void val(const String& s,int& result,int& code) {
    try {std::size_t n;result=std::stoi(s.value,&n);code=n==s.value.size()?0:static_cast<int>(n+1);}
    catch(...){result=0;code=1;}
}
template<class T> auto abs(T v) { return std::abs(v); }
inline int round(double v) { return static_cast<int>(std::nearbyint(v)); }
inline int floor(double v) { return static_cast<int>(std::floor(v)); }
inline double sqrt(double v) { return std::sqrt(v); }

// Delphi System.Random: 32-bit LCG and high word of the 64-bit range product.
inline std::uint32_t randseed=0;
inline int random(int range) {
    randseed=randseed*0x08088405u+1u;
    return static_cast<int>((std::uint64_t(randseed)*std::uint32_t(range))>>32);
}
inline std::uint64_t delphi_hash_random() {
    const std::uint64_t c=random(65536)+8192;
    const std::uint64_t b=random(214748367)+1;
    const std::uint64_t a=random(214748367)+1;
    return (a*b*c)<<1;
}
inline std::int32_t gettickcount() {
    return static_cast<std::int32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
}
inline std::function<void(const String&)> output;
inline void send_winboard(const String& s) { if(output)output(s); }
inline void showmessage(const String& s) { std::cerr<<s.value<<'\n'; }
struct StringList {};
struct Form { String defaultdir="books"; String caption; };
inline Form form1;

// Only book and legacy PGN routines use Pascal file operations.
struct File { std::filesystem::path name; std::fstream stream; };
inline int last_io_result=0;
inline std::filesystem::path native_path(const String& s) {
    std::string p=s.value;for(char& c:p)if(c=='\\')c='/';return p;
}
inline bool fileexists(const String& s) { return std::filesystem::exists(native_path(s)); }
inline void assignfile(File& f,const String& name) { f.name=native_path(name); }
inline void reset(File& f,int=0) {
    f.stream.open(f.name,std::ios::in|std::ios::binary);last_io_result=f.stream?0:1;
    if(last_io_result)throw std::runtime_error("Cannot open "+f.name.string());
}
inline void rewrite(File& f) {
    f.stream.open(f.name,std::ios::out|std::ios::binary|std::ios::trunc);last_io_result=f.stream?0:1;
    if(last_io_result)throw std::runtime_error("Cannot write "+f.name.string());
}
inline int ioresult() {int r=last_io_result;last_io_result=0;return r;}
template<class T> void blockread(File& f,T& buffer,std::size_t bytes,int& got) {f.stream.read(reinterpret_cast<char*>(&buffer),bytes);got=static_cast<int>(f.stream.gcount());}
template<class T> void write(File& f,const T& v,std::size_t bytes) {f.stream.write(reinterpret_cast<const char*>(&v),bytes);if(!f.stream)throw std::runtime_error("Book write failed: "+f.name.string());}
inline void closefile(File& f) { f.stream.close(); }
inline bool eof(File& f) { return f.stream.peek()==std::char_traits<char>::eof(); }
inline void readln(File& f,String& s) { std::getline(f.stream,s.value);if(!s.value.empty() && s.value.back()=='\r')s.value.pop_back(); }
}
