#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <string>
#include <utility>
#include <concepts>
#include <list>
#include <array>
#include <vector>

class Timer
{
private:
    std::chrono::duration<long long, std::ratio<1, 1000000>> dr;
    std::chrono::time_point<std::chrono::system_clock> be;
public:
    Timer() : dr(0) {};

    void start()
    {
        be = std::chrono::system_clock::now();
    }

    void stop()
    {
        auto v = std::chrono::system_clock::now() - be;
        dr += v;
    }

    double microsecond()
    {
        return ((double) std::chrono::duration_cast<std::chrono::microseconds>(dr).count());
    }
};

class Hashable
{
public:
    virtual uint32_t hash() const = 0;
};

class Element : public Hashable
{
private:
    static const uint32_t z = 11;
    std::string s;
public:

    Element(std::string s) : s(std::move(s)) {};

    uint32_t hash() const override
    {
        size_t l = s.length();
        uint32_t h = 0;
        for (int i = 0; i < l; i += 4)
        {
            if (l - i < 4)
            {
                uint32_t v = (uint32_t) s[i] << 24;
                if (i + 1 < l)
                {
                    v += (uint32_t) s[i + 1] << 16;
                }
                if (i + 2 < l)
                {
                    v += (uint32_t) s[i + 2] << 8;
                }
                h = h * z + v;
            }
            else
            {
                uint32_t v = *((uint32_t *) (s.c_str() + i));
                h = h * z + v;
            }
        }
        return h;
    }

    bool operator==(const Element &e) const
    {
        return this->s == e.s;
    }
};

class Simple
{
public:
    u_int32_t operator()(const uint32_t &n, const uint32_t &x) const
    {
        return x % n;
    }

    static std::string name()
    {
        return "Simple";
    }
};

class Multi
{
private:
    static constexpr double A = 0.6180339887498949;
public:
    u_int32_t operator()(const uint32_t &n, const uint32_t &x) const
    {
        return (uint32_t) (n * (x * A - (double) ((uint32_t) (x * A))));
    }

    static std::string name()
    {
        std::ostringstream os;
        os << "Multiplication Method with A=" << A;
        return os.str();
    }
};

template<uint32_t P = 990523>
class Division
{
public:
    u_int32_t operator()(const uint32_t &n, const uint32_t &x) const
    {
        return x % P % n;
    }

    static std::string name()
    {
        std::ostringstream os;
        os << "Division Method with P=" << P;
        return os.str();
    }
};

template<uint32_t a = 233333, uint32_t b = 114514, uint32_t P = 990523> requires (0 < a && a < P && b < P)
class MAD
{
public:
    u_int32_t operator()(const uint32_t &n, const uint32_t &x) const
    {
        return (a * x + b) % P % n;
    }

    static std::string name()
    {
        std::ostringstream os;
        os << "Division Method with a=" << a << " b=" << b << " P=" << P;
        return os.str();
    }
};

template<class T>
concept HashClass = std::is_base_of<Hashable, T>::value;

template<HashClass T, class CompFunc, uint32_t n>
requires
requires(CompFunc cp, uint32_t x) {
    cp(n, x);
}
class HashSet
{
private:
    std::array<std::list<T>, n> a;
    CompFunc cf;
    int put_count = 0;
    int collision_count = 0;
public:
    bool put(const T &e)
    {
        put_count++;
        auto x = e.hash();
        auto w = cf(n, x);
        if (!a[w].empty())
        {
            collision_count++;
        }
        auto it = std::find(a[w].begin(), a[w].end(), e);
        if (it != a[w].end())
        {
            return false;
        }
        else
        {
            a[w].push_back(e);
            return true;
        }
    }

    bool find(const T &e)
    {
        auto x = e.hash();
        auto w = cf(n, x);
        auto it = std::find(a[w].begin(), a[w].end(), e);
        if (it != a[w].end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool remove(const T &e)
    {
        auto x = e.hash();
        auto w = cf(n, x);
        auto it = std::find(a[w].begin(), a[w].end(), e);
        if (it != a[w].end())
        {
            a[w].erase(it);
            return true;
        }
        else
        {
            return false;
        }
    }

    double collision()
    {
        return (double) collision_count / put_count;
    }

};

int main()
{
    typedef MAD<> CmpFunc;
    std::string filename = "HW3-input-names.txt";
    std::ifstream input;
    input.open(filename);
    std::vector<Element> data;
    std::string s;
    while (std::getline(input, s))
    {
        data.emplace_back(s);
    }
    HashSet<Element, CmpFunc, 4000> h4k;
    HashSet<Element, CmpFunc, 8000> h8k;
    HashSet<Element, CmpFunc, 16000> h16k;
    Timer t4k, t8k, t16k;
    for (auto e: data)
    {
        t4k.start();
        h4k.put(e);
        t4k.stop();
        t8k.start();
        h8k.put(e);
        t8k.stop();
        t16k.start();
        h16k.put(e);
        t16k.stop();
    }
    std::cout << "File: " << filename << std::endl;
    std::cout << "The h2() function is " << CmpFunc::name() << "." << std::endl;
    std::cout << "The collicion persentage of size 4000 is " << h4k.collision() << "." << std::endl;
    std::cout << "The average search time of size 4000 is " << t4k.microsecond() / data.size() << "ms." << std::endl;
    std::cout << "The collicion persentage of size 8000 is " << h8k.collision() << "." << std::endl;
    std::cout << "The average search time of size 8000 is " << t8k.microsecond() / data.size() << "ms." << std::endl;
    std::cout << "The collicion persentage of size 16000 is " << h16k.collision() << "." << std::endl;
    std::cout << "The average search time of size 16000 is " << t16k.microsecond() / data.size() << "ms." << std::endl;
    return 0;
}
