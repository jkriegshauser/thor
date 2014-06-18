#include "gtest/gtest.h"

#include "../basic_string.h"

#define STR(a) (sizeof(string::value_type) == 1 ? (string::const_pointer)a : (string::const_pointer)L##a)

template <typename string> void string_test()
{
    {
        // Default construct
        string s;
        EXPECT_TRUE(s.empty());
        EXPECT_EQ(s.capacity(), 0);
        EXPECT_EQ(s.max_capacity(), string::npos);
        EXPECT_EQ(s.size(), 0);
        EXPECT_EQ(s.length(), 0);
        EXPECT_TRUE(s.c_str() != 0);
        EXPECT_TRUE(s.data() != 0);
        EXPECT_EQ(*s.c_str(), '\0');
        EXPECT_TRUE(s.begin() == s.end());
        EXPECT_TRUE(s.cbegin() == s.cend());
        EXPECT_TRUE(s.rbegin() == s.rend());
        EXPECT_TRUE(s.crbegin() == s.crend());
    }
    {
        // Pointer construct
        string s(STR("This is a test."));
        EXPECT_TRUE(!s.empty());
        EXPECT_TRUE(s.capacity() >= 15);
        EXPECT_TRUE(s.length() == 15);
        EXPECT_STREQ(s.data(), STR("This is a test."));

        // Iterator tests
        {
            string::iterator first, last;
            first = s.begin(), last = s.end();
            EXPECT_TRUE(first == s.begin());
            EXPECT_TRUE(first != last);
            EXPECT_TRUE(first < last);
            EXPECT_TRUE(last > first);
            EXPECT_TRUE(first <= last);
            EXPECT_FALSE(first >= last);
            string::const_iterator cfirst(first), clast;
            clast = last;

            EXPECT_EQ(thor::distance(first, last), 15);
            EXPECT_EQ(last - first, 15);
            EXPECT_EQ(*first, string::value_type('T'));
            EXPECT_TRUE(first++ == s.begin());
            EXPECT_EQ(*first, string::value_type('h'));
            EXPECT_EQ(*++first, string::value_type('i'));
            *first = string::value_type('I');
            first -= 2;
            EXPECT_TRUE(first == s.begin());
            first += 15;
            EXPECT_TRUE(first == s.end());
            first = first - 15;
            EXPECT_TRUE(first == s.begin());
            last = first + 15;
            EXPECT_TRUE(last == s.end());

            EXPECT_STREQ(s.c_str(), STR("ThIs is a test."));
            *(s.begin() + 2) = string::value_type('i');
        }
        // Const Iterator tests
        {
            string::const_iterator first, last;
            first = s.cbegin(), last = s.cend();
            EXPECT_TRUE(first == s.begin());
            EXPECT_TRUE(first != last);
            EXPECT_TRUE(first < last);
            EXPECT_TRUE(last > first);
            EXPECT_TRUE(first <= last);
            EXPECT_FALSE(first >= last);
            string::const_iterator cfirst(first), clast;
            clast = last;

            EXPECT_EQ(thor::distance(first, last), 15);
            EXPECT_EQ(last - first, 15);
            EXPECT_EQ(*first, string::value_type('T'));
            EXPECT_TRUE(first++ == s.begin());
            EXPECT_EQ(*first, string::value_type('h'));
            EXPECT_EQ(*++first, string::value_type('i'));
            // *first = string::value_type('I'); // Correctly won't compile
            first -= 2;
            EXPECT_TRUE(first == s.begin());
            first += 15;
            EXPECT_TRUE(first == s.end());
            first = first - 15;
            EXPECT_TRUE(first == s.begin());
            last = first + 15;
            EXPECT_TRUE(last == s.end());
        }
        // Reverse Iterator tests
        {
            string::reverse_iterator first, last;
            first = s.rbegin(), last = s.rend();
            EXPECT_TRUE(first == s.rbegin());
            EXPECT_TRUE(first != last);
            //TODO: busted
            //EXPECT_TRUE(first < last);
            //EXPECT_TRUE(last > first);
            //EXPECT_TRUE(first <= last);
            //EXPECT_FALSE(first >= last);
            string::const_reverse_iterator cfirst(first), clast;
            clast = last;

            EXPECT_EQ(thor::distance(first, last), 15);
            EXPECT_EQ(last - first, 15);
            EXPECT_EQ(*first, string::value_type('.'));
            EXPECT_TRUE(first++ == s.rbegin());
            EXPECT_EQ(*first, string::value_type('t'));
            EXPECT_EQ(*++first, string::value_type('s'));
            *first = string::value_type('S');
            first -= 2;
            EXPECT_TRUE(first == s.rbegin());
            first += 15;
            EXPECT_TRUE(first == s.rend());
            first = first - 15;
            EXPECT_TRUE(first == s.rbegin());
            last = first + 15;
            EXPECT_TRUE(last == s.rend());

            EXPECT_STREQ(s.c_str(), STR("This is a teSt."));
            *(s.rbegin() + 2) = string::value_type('s');
            EXPECT_STREQ(s.c_str(), STR("This is a test."));
        }
        // Const Reverse Iterator tests
        {
            string::const_reverse_iterator first, last;
            first = s.crbegin(), last = s.crend();
            EXPECT_TRUE(first == s.rbegin());
            EXPECT_TRUE(first != last);
            // TODO: Busted
            //EXPECT_TRUE(first < last);
            //EXPECT_TRUE(last > first);
            //EXPECT_TRUE(first <= last);
            //EXPECT_FALSE(first >= last);
            string::const_reverse_iterator cfirst(first), clast;
            clast = last;

            EXPECT_EQ(thor::distance(first, last), 15);
            EXPECT_EQ(last - first, 15);
            EXPECT_EQ(*first, string::value_type('.'));
            EXPECT_TRUE(first++ == s.rbegin());
            EXPECT_EQ(*first, string::value_type('t'));
            EXPECT_EQ(*++first, string::value_type('s'));
            // *first = string::value_type('S'); // Correctly won't compile
            first -= 2;
            EXPECT_TRUE(first == s.rbegin());
            first += 15;
            EXPECT_TRUE(first == s.rend());
            first = first - 15;
            EXPECT_TRUE(first == s.rbegin());
            last = first + 15;
            EXPECT_TRUE(last == s.rend());
        }

        // copy constructor
        string s2(s);
        EXPECT_EQ(15, s2.length());

        // substring constructor
        string s3(s, 10);
        EXPECT_EQ(5, s3.length());
        EXPECT_STREQ(s3.c_str(), STR("test."));
        string s4(s, 5, 2);
        EXPECT_EQ(2, s4.length());
        EXPECT_STREQ(STR("is"), s4.c_str());

        // buffer constructor
        string::value_type buffer[] = { 0, 1, 2, 3, 4, 5, 6 };
        string s5(buffer, 5);
        EXPECT_EQ(5, s5.length());
        EXPECT_STREQ(s5.c_str(), STR(""));
        EXPECT_EQ(s5[4], string::value_type(4));

        // fill constructor
        string s6(10, string::value_type('!'));
        EXPECT_EQ(10, s6.length());
        EXPECT_STREQ(STR("!!!!!!!!!!"), s6.c_str());

        // iterator constructor
        const string::value_type* p = STR("This is an iterator test.");
        string s7(p + 11, p + 19);
        EXPECT_EQ(8, s7.length());
        EXPECT_STREQ(STR("iterator"), s7.c_str());

        // formatting constructor
        string s8(string::fmt, STR("%d%d%c"), 999, 888, string::value_type('!'));
        EXPECT_STREQ(STR("999888!"), s8.c_str());
        EXPECT_EQ(7, s8.length());

        // literal string constructor
        string s9(string::lit_allow_share, p);
        EXPECT_TRUE(s9.c_str() == p);
        EXPECT_EQ(s9.length(), 25);
        EXPECT_EQ(s9.capacity(), string::npos); // shareable literal strings use npos

        string s10(string::lit_copy_on_share, p);
        EXPECT_TRUE(s10.c_str() == p);
        EXPECT_EQ(s10.length(), 25);
        EXPECT_EQ(s10.capacity(), 0);

        string s11(s10);
        EXPECT_TRUE(s11.c_str() != p);
        EXPECT_EQ(s11.length(), 25);
        EXPECT_TRUE(s11.capacity() + 1 > 1); // should be a copy
    }

    {
        // Element access (const)
        const string s(STR("Element access test."));
        const string s2(s); // Hold a shared reference
        
        EXPECT_EQ(s[0], string::value_type('E'));
        EXPECT_TRUE(s2.c_str() == s.c_str() || THOR_SUPPRESS_WARNING(string::embedded_size != 0));
        EXPECT_EQ(s[s.length()], 0);
        EXPECT_TRUE(s2.c_str() == s.c_str() || THOR_SUPPRESS_WARNING(string::embedded_size != 0));
        EXPECT_EQ(s.at(1), string::value_type('l'));
        EXPECT_TRUE(s2.c_str() == s.c_str() || THOR_SUPPRESS_WARNING(string::embedded_size != 0));
        EXPECT_EQ(s.at(s.length()), 0);
        EXPECT_TRUE(s2.c_str() == s.c_str() || THOR_SUPPRESS_WARNING(string::embedded_size != 0));
        EXPECT_EQ(s.front(), 'E');
        EXPECT_TRUE(s2.c_str() == s.c_str() || THOR_SUPPRESS_WARNING(string::embedded_size != 0));
        EXPECT_EQ(s.back(), '.');
        EXPECT_TRUE(s2.c_str() == s.c_str() || THOR_SUPPRESS_WARNING(string::embedded_size != 0));
    }
    // Element access (mutable)
    {
        string s(STR("Element access test.")), s2(s);
        EXPECT_TRUE(s.c_str() == s2.c_str() || THOR_SUPPRESS_WARNING(string::embedded_size != 0)); // sharing
        s[0] = 'e';
        EXPECT_STREQ(STR("element access test."), s.c_str());
        EXPECT_FALSE(s.c_str() == s2.c_str()); // no longer sharing
    }
    {
        string s(STR("Element access test.")), s2(s);
        EXPECT_TRUE(s.c_str() == s2.c_str() || THOR_SUPPRESS_WARNING(string::embedded_size != 0)); // sharing
        s[s.length() - 1] = '!';
        EXPECT_STREQ(STR("Element access test!"), s.c_str());
        EXPECT_FALSE(s.c_str() == s2.c_str()); // no longer sharing
    }
    {
        string s(STR("Element access test.")), s2(s);
        EXPECT_TRUE(s.c_str() == s2.c_str() || THOR_SUPPRESS_WARNING(string::embedded_size != 0)); // sharing
        s.at(0) = 'e';
        EXPECT_STREQ(STR("element access test."), s.c_str());
        EXPECT_FALSE(s.c_str() == s2.c_str()); // no longer sharing
    }
    {
        string s(STR("Element access test.")), s2(s);
        EXPECT_TRUE(s.c_str() == s2.c_str() || THOR_SUPPRESS_WARNING(string::embedded_size != 0)); // sharing
        s.at(s.length() - 1) = '!';
        EXPECT_STREQ(STR("Element access test!"), s.c_str());
        EXPECT_FALSE(s.c_str() == s2.c_str()); // no longer sharing
    }
    {
        string s(STR("Element access test.")), s2(s);
        EXPECT_TRUE(s.c_str() == s2.c_str() || THOR_SUPPRESS_WARNING(string::embedded_size != 0)); // sharing
        s.front() = 'e';
        EXPECT_STREQ(STR("element access test."), s.c_str());
        EXPECT_FALSE(s.c_str() == s2.c_str()); // no longer sharing
    }
    {
        string s(STR("Element access test.")), s2(s);
        EXPECT_TRUE(s.c_str() == s2.c_str() || THOR_SUPPRESS_WARNING(string::embedded_size != 0)); // sharing
        s.back() = '!';
        EXPECT_STREQ(STR("Element access test!"), s.c_str());
        EXPECT_FALSE(s.c_str() == s2.c_str()); // no longer sharing
    }

    // Operators
    {
        string s;
        s = string(STR("hello world"));
        EXPECT_STREQ(STR("hello world"), s.c_str());
        EXPECT_EQ(s.length(), 11);
        if (THOR_SUPPRESS_WARNING(string::embedded_size == 0))
        {
            EXPECT_TRUE(s.c_str() == s.c_str()); // sharing
        }

        string s2(s); // Force original string shared if possible
        s += string(STR("!!1!"));
        EXPECT_STREQ(STR("hello world!!1!"), s.c_str());
        EXPECT_EQ(15, s.length());

        s2 = s;
        string s3 = s + string(STR("????"));
        EXPECT_STREQ(STR("hello world!!1!????"), s3.c_str());
        EXPECT_EQ(19, s3.length());
    }
    {
        string s;
        s = STR("hello world");
        EXPECT_STREQ(STR("hello world"), s.c_str());
        EXPECT_EQ(s.length(), 11);

        string s2(s); // Force original string shared if possible
        s += STR("!!1!");
        EXPECT_STREQ(STR("hello world!!1!"), s.c_str());
        EXPECT_EQ(15, s.length());

        s2 = s;
        string s3 = s + STR("????");
        EXPECT_STREQ(STR("hello world!!1!????"), s3.c_str());
        EXPECT_EQ(19, s3.length());
    }
    {
        string s;
        s = string::value_type('?');
        EXPECT_STREQ(STR("?"), s.c_str());
        EXPECT_EQ(s.length(), 1);

        string s2(s); // Force original string shared if possible
        s += string::value_type('!');
        EXPECT_STREQ(STR("?!"), s.c_str());
        EXPECT_EQ(s.length(), 2);

        string s3 = s + string::value_type('@');
        EXPECT_STREQ(STR("?!@"), s3.c_str());
        EXPECT_EQ(s3.length(), 3);
    }

    // TODO: Appending
    // TODO: Assigning
    // TODO: Insertion
    // TODO: Erasure
    // TODO: Replacement
    // TODO: Swap
    // TODO: Copy/Substr
    // TODO: Find
    {
        string f(STR("This is a VERY long string! It's so incredibly long. Can you believe it?"));
        string s(STR("the quick brown fox jumped over the lazy dog.\0dog."), 50);
        EXPECT_EQ(string::npos, s.find(f));
        EXPECT_EQ(32, s.find(string(STR("the")), 1));
        EXPECT_EQ(45, s.find(string(STR("\0dog"), 4), 0));
        EXPECT_EQ(4, s.find(STR("quick"), 0));
        EXPECT_EQ(32, s.find(string::value_type('t'), 1));

        EXPECT_EQ(string::npos, s.rfind(f));
        EXPECT_EQ(46, s.rfind(string(STR("dog"))));
        EXPECT_EQ(43, s.rfind(STR("g"), s.length() - 2));
        EXPECT_EQ(43, s.rfind(string::value_type('g'), s.length() - 2));

        EXPECT_EQ(string::npos, s.find_i(f));
        EXPECT_EQ(32, s.find_i(string(STR("The")), 1));
        EXPECT_EQ(45, s.find_i(string(STR("\0DoG"), 4), 0));
        EXPECT_EQ(4,  s.find_i(STR("QUICK"), 0));
        EXPECT_EQ(32, s.find_i(string::value_type('T'), 1));

        EXPECT_EQ(string::npos, s.rfind_i(f));
        EXPECT_EQ(46, s.rfind_i(string(STR("DoG"))));
        EXPECT_EQ(43, s.rfind_i(STR("G"), s.length() - 2));
        EXPECT_EQ(43, s.rfind_i(string::value_type('G'), s.length() - 2));
    }
    // TODO: Compare


    // Hash test
    thor::hash<string> strhash;
    const thor::size_type expected = thor::hash<const string::value_type*>()(STR("This is a test"));
    string s(STR("This is a test"));
    const thor::size_type actual = strhash(s);
    EXPECT_EQ(expected, actual);
}

TEST(strings, c_string)
{
    string_test<thor::string>();
}

TEST(strings, wide_string)
{
    string_test<thor::wstring>();
}

TEST(strings, c_string_fixed)
{
    string_test<thor::basic_string<char, 32> >();
}

TEST(strings, wide_string_fixed)
{
    string_test<thor::basic_string<wchar_t, 32> >();
}

TEST(strings, old)
{
    thor::string str;
    EXPECT_TRUE(str.empty());
    EXPECT_TRUE(str.c_str() != 0);
    EXPECT_TRUE(str.data() != 0);
    EXPECT_TRUE(*str.c_str() == '\0');
    EXPECT_TRUE(str.begin() == str.end());
    EXPECT_TRUE(str.cbegin() == str.cend());
    EXPECT_TRUE(str.rbegin() == str.rend());
    EXPECT_TRUE(str.crbegin() == str.crend());

    thor::string str2(str);
    thor::string str3("hello world");
    thor::string str4(5, 'h');
    thor::string str6(str4);

    const char* p = "HELLO world";
    thor::string str5(p, p + 5);
    str5.resize(1);
    EXPECT_TRUE(str5.length() == 1);
    str5.resize(5, 'C');
    EXPECT_TRUE(str5.length() == 5);
    thor::string str7(str5, 2);
    EXPECT_TRUE(str7.length() == 3);
    EXPECT_TRUE(str7 == "CCC");
    str7.reserve(100);
    EXPECT_TRUE(str7.capacity() >= 100);

    str3[0] = 'H';
    EXPECT_TRUE(str3 == "Hello world");
    str3.at(1) = 'E';
    EXPECT_TRUE(str3 == "HEllo world");
    str3.front() = 'h';
    EXPECT_TRUE(str3 == "hEllo world");
    str3.back() = 'D';
    EXPECT_TRUE(str3 == "hEllo worlD");

    str3.append(thor::string(" too!"));
    EXPECT_TRUE(str3 == "hEllo worlD too!");

    str6.push_back('j');
    EXPECT_STREQ("hhhhhj", str6.c_str());
    EXPECT_STREQ("hhhhh", str4.c_str());

    str6.push_back();
    EXPECT_TRUE(str6.length() == 7);

    str3.append(" too!");
    EXPECT_STREQ("hEllo worlD too! too!", str3.c_str());

    str4.append(5, 'j');
    EXPECT_STREQ("hhhhhjjjjj", str4.c_str());

    str4.append(p, p + 5);
    EXPECT_STREQ("hhhhhjjjjjHELLO", str4.c_str());

    str4.insert(5, thor::string(5, 'i'));
    EXPECT_STREQ("hhhhhiiiiijjjjjHELLO", str4.c_str());
    EXPECT_EQ(20, str4.length());

    str7.insert(1, thor::string(5, 'c'), 2);
    EXPECT_STREQ("CcccCC", str7.c_str());
    EXPECT_EQ(6, str7.length());

    str7.insert(1, "test");
    EXPECT_STREQ("CtestcccCC", str7.c_str());
    EXPECT_EQ(10, str7.length());

    str7.insert(5, "test harness", 4);
    EXPECT_STREQ("CtesttestcccCC", str7.c_str());
    EXPECT_EQ(14, str7.length());

    str7.insert(1, 4, 'c');
    EXPECT_STREQ("CcccctesttestcccCC", str7.c_str());
    EXPECT_EQ(18, str7.length());

    str6[6] = 'j';
    str6.insert(str6.end(), 2, 'i');
    EXPECT_STREQ("hhhhhjjii", str6.c_str());
    str6.insert(str6.end(), 'i');
    EXPECT_STREQ("hhhhhjjiii", str6.c_str());
    str6.insert(str6.begin(), p + 6, p + 11);
    EXPECT_STREQ("worldhhhhhjjiii", str6.c_str());

    str6.erase(5, 5);
    EXPECT_STREQ("worldjjiii", str6.c_str());

    str6.erase();
    EXPECT_EQ(0, str6.length());

    thor::string::iterator iter = str7.erase(str7.begin() + 5);
    EXPECT_STREQ("CccccesttestcccCC", str7.c_str());
    EXPECT_EQ(*iter, 'e');

    iter = str7.erase(str7.begin() + 5, str7.begin() + 8);
    EXPECT_STREQ("CcccctestcccCC", str7.c_str());
    EXPECT_EQ(*iter, 't');

    char c = str7.pop_back();
    EXPECT_STREQ("CcccctestcccC", str7.c_str());
    EXPECT_EQ(c, 'C');

    thor::string fmttest(thor::string::fmt, "%s", "This is a test!");
    EXPECT_STREQ("This is a test!", fmttest.c_str());
    EXPECT_EQ(15, fmttest.length());
    fmttest.append_format("%d%s", 0, "Booya!");
    EXPECT_STREQ("This is a test!0Booya!", fmttest.c_str());
    EXPECT_EQ(22, fmttest.length());
    fmttest.insert_format(2, "%c%c", 'i', 'i');
    EXPECT_STREQ("Thiiis is a test!0Booya!", fmttest.c_str());
    EXPECT_EQ(24, fmttest.length());

    fmttest.replace(2, 3, thor::string(1, 'i')); // Shrink
    EXPECT_STREQ("This is a test!0Booya!", fmttest.c_str());
    EXPECT_EQ(22, fmttest.length());
    fmttest.replace(10, 4, thor::string("examination")); // Growth
    EXPECT_STREQ("This is a examination!0Booya!", fmttest.c_str());
    EXPECT_EQ(29, fmttest.length());
    fmttest.replace(0, 4, thor::string("THIS")); // Equal
    EXPECT_STREQ("THIS is a examination!0Booya!", fmttest.c_str());
    EXPECT_EQ(29, fmttest.length());

    // Iterator version
    fmttest = "This is a test.";
    fmttest.replace(fmttest.begin() + 9, fmttest.begin() + 14, thor::string("n examination"));
    EXPECT_STREQ("This is an examination.", fmttest.c_str());
    EXPECT_EQ(23, fmttest.length());
    fmttest.replace(fmttest.begin() + 4, fmttest.begin() + 7, thor::string("'s"));
    EXPECT_STREQ("This's an examination.", fmttest.c_str());
    EXPECT_EQ(22, fmttest.length());
    fmttest.replace(fmttest.begin(), fmttest.begin() + 4, thor::string("THIS"));
    EXPECT_STREQ("THIS's an examination.", fmttest.c_str());
    EXPECT_EQ(22, fmttest.length());

    // Replace with fill
    fmttest = "This is a test.";
    fmttest.replace(fmttest.begin() + 9, fmttest.begin() + 14, 13, '?');
    EXPECT_STREQ("This is a?????????????.", fmttest.c_str());
    EXPECT_EQ(23, fmttest.length());
    fmttest.replace(fmttest.begin() + 4, fmttest.begin() + 7, 2, '-');
    EXPECT_STREQ("This-- a?????????????.", fmttest.c_str());
    EXPECT_EQ(22, fmttest.length());
    fmttest.replace(fmttest.begin(), fmttest.begin() + 4, 4, '*');
    EXPECT_STREQ("****-- a?????????????.", fmttest.c_str());
    EXPECT_EQ(22, fmttest.length());

    // Replace with substring
    fmttest = "This is a test.";
    fmttest.replace(10, 4, thor::string("This is an expression."), 11U, 10U);
    EXPECT_STREQ("This is a expression.", fmttest.c_str());
    EXPECT_EQ(21, fmttest.length());
    
    // Replace with input iterators
    fmttest = "This is a test.";
    fmttest.replace(fmttest.begin() + 10, fmttest.begin() + 14, p, p + 5);
    EXPECT_STREQ("This is a HELLO.", fmttest.c_str());
    EXPECT_EQ(16, fmttest.length());
    
    // Replace with format
    fmttest = "This is a test.";
    fmttest.replace_format(fmttest.begin() + 10, fmttest.begin() + 14, "%d%s", 0, "37");
    EXPECT_STREQ("This is a 037.", fmttest.c_str());
    EXPECT_EQ(14, fmttest.length());

    thor::string substr_test = thor::string("This is a test.").substr(10, 4);
    EXPECT_STREQ("test", substr_test.c_str());
    EXPECT_EQ(4, substr_test.length());

    char buffer[6] = {0};
    fmttest = "This is a test.";
    fmttest.copy(buffer, 4, 10);
    EXPECT_STREQ("test", buffer);

    thor::string::size_type where = fmttest.find("supercalifragilisticexpialidocious");
    EXPECT_EQ(thor::string::npos, where);
    where = fmttest.find("test");
    EXPECT_EQ(10, where);
    where = fmttest.find("test.");
    EXPECT_EQ(10, where);
    where = fmttest.find('.');
    EXPECT_EQ(14, where);
    where = fmttest.rfind('.');
    EXPECT_EQ(14, where);
    where = fmttest.rfind('.', 10);
    EXPECT_EQ(thor::string::npos, where);
    where = fmttest.rfind("t.");
    EXPECT_EQ(13, where);
    where = fmttest.rfind("Th", 10);
    EXPECT_EQ(0, where);
    where = fmttest.find_first_of("aeiou");
    EXPECT_EQ(2, where);
    where = fmttest.find_first_not_of("This");
    EXPECT_EQ(4, where);
    where = fmttest.find_last_of("aeiou");
    EXPECT_EQ(11, where);
    where = fmttest.find_last_not_of(". ", 10);
    EXPECT_EQ(8, where);

    thor::basic_string<char, 16> fixed, fixed2;
    fixed = fixed2 = "hello world";
    EXPECT_EQ(16, fixed.capacity());
    EXPECT_EQ(11, fixed.length());
    thor::string notfixed = fixed;
    EXPECT_EQ(11, notfixed.length());
    EXPECT_NE(notfixed.c_str(), fixed.c_str());
    fixed += notfixed;
    EXPECT_EQ(22, fixed.length());

    fixed2.swap(notfixed);

    thor::string notfixed2 = notfixed;
    EXPECT_EQ(notfixed.c_str(), notfixed2.c_str());
    EXPECT_EQ(notfixed.length(), notfixed2.length());
    EXPECT_STREQ(notfixed.c_str(), notfixed2.c_str());

    notfixed.swap(notfixed2);
}
