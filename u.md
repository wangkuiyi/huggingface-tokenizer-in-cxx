# Everything C++ and Python Programmers Need to Know about Unicode

## Unicode

Unicode is larger character set than ASCII.  It contains codepoints for non-English characters. A [codepoint](https://en.wikipedia.org/wiki/Code_point) is a numberic value that maps to a specific [character](https://en.wikipedia.org/wiki/Character_(computing)). A character is a symbol we draw on screens and papers.

Unlike an ASCII codepoint ranges from 0 to 127 and takes a byte with high-bit fixed to 0, or a `char`, Unicode has more codepoints that we will need four bytes, or a `uint32_t`, or `wchar_t`, to save each codepoint.

## UTF-8

Most characters in Unicode are infrequent.  Inspired by Huffman coding, we want to use less than four bytes for those frequent codepoints.  This idea leads to a well-known encoding algorithm, UTF-8, which encodes each Unicode codepoint into one, two, three, or four bytes.

<img width="738" alt="Screenshot 2023-02-07 at 5 08 03 PM" src="https://user-images.githubusercontent.com/1548775/217402628-3f7c2d96-d2bb-41aa-a581-040193eb298d.png">

## One-byte UTF-8 Codepoints

Each one-byte UTF-8 encoded codepoint corresponds to an ASCII codepoint.

<img width="738" src="https://web.alfredstate.edu/faculty/weimandn/miscellaneous/ascii/ASCII%20Conversion%20Chart.gif" />

In the above ASCII table, the character `$` has the codepoint with decimal value 37 and, equivalently, the binary value 00100100, where the high-bit is 0.

## Multiple-byte UTF-8 Codepoints

When we use a modern text editor to type C++ source code, all string literals are encoded in UTF-8.

The following source code assigns a UTF-8 encoded string to a `std::string`-typed variable `text`.

```c++
std::string text = "Hi 益";
```

The first three characters, `H`, `i` and ` `, are in ASCII.  So each of them takes one byte in an UTF-8 encoded string.  We are curious to know how many bytes the characters `益` takes in this UTF-8 string.

Let us do the following anatomy.

```c++
strlen(s.c_str()) // returns 6
s.size()          // returns 6
```

Because `std::string` is an alias of `basic_string<char>`, which represents a sequence of `char`s.  In the above example, the character `益` must take 6-3=3 bytes.

The following code snippet prints each of these six bytes:

```c++
  for(size_t i = 0; i < text.size(); ++i) {
    std::cout << " " << static_cast<unsigned int>(static_cast<unsigned char>(text[i]));
  }
```

It gives us:

```
72 105 32 231 155 138
```

Look up the above ASCII table, we can see 

- 72 is `H`
- 105 is `i`
- 32 is ` ` (the whitespace)

Converting the decimal values of the rest three bytes into binary, we get:

```
231 = 11100111
155 = 10011011
138 = 10001010
```

Looking up the above UTF-8 encoding table, the three prefixes, 1110, 10, and 10, match the case of a three-byte codepoint.  So, we remove the three prefixes and concatenate the rest.

```
0111 011011 001010
```

Converting this binary number into decimal, we get

30410

Google tells us 30410 **is** the codepoint of `益`!

## From `char` to `wchar_t`

Given that the character `益` takes a value that requires more than one byte to store, and the C/C++ type `char` takes only one byte, it is illegal to assign a multi-byte Unicode codepoint to a `char`-typed variables.  The following line of code will trigger a compilation error.

```c++
char c = '益'; // character too large for enclosing character literal type
```

Therefore, to fully support Unicode, C/C++ needs a new character type, which takes four bytes -- the longest UTF-8 codepoint takes four bytes.  This is `wchar_t`.

However, changing only `char` into `wchar_t` is not enough.  The following code give us the same error.

```
wchar_t c =  '益';  // character too large for enclosing character literal type
```

This is because the literal syntax (`' '`) is for defining a `char` rather than a `wchar_t`.  We need the `L`-prefix to denote that each character in the string takes four bytes.

```
wchar_t c = L'益';  // This works!
```

And we have `sizeof(c) == 4` because there are four Unicode codepoitns in the string.

We have `sizeof(c[0])==4` on Linux, macOS, and other POSIX-compatible systems. It is 2 on Windows due to a POSIX-incompatibility issue.

## From `std::string` to `std::wstring`

| string type  | implementation | literal syntax |
| ------------- | ------------- | -------------- |
| `std::string`  | `/* std::basic_string<char> */`  | `s = "Hi 益"` |
| `std::wstring`  | `/* std::basic_string<wchar_t> */` | `w = L'Hi益"` |

We have `s.size()==6` because the UTF-i encoding of `"Hi 益"` takes 6 bytes. We have `w.size()==4` because it contains 4 Unicode codepoints.

The following code snippet prints each codepoint.

```c++
  for(size_t i = 0; i < text.size(); ++i) {
    std::cout << " " << static_cast<uint32_t>(text[i]);
  }
```

and gives

```
72 105 32 30410
```

We do not have to manually decoding it by looking up the UTF-8 prefix table.

The following functions encode wstring in UTF-8 and decode UTF-8 encoded char-string to wstring.

```c++
#include <string>
#include <codecvt>

std::wstring utf8_to_wstring (const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.from_bytes(str);
}

std::string wstring_to_utf8 (const std::wstring& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.to_bytes(str);
}
```

## From C++ to Python

Python has two data types `str` and `bytes`.  A string literal has type `str`, which behave like `std::wstring` in terms that `len(s:str)` returns the number of Unicode codepoints.  By UTF-8 encoding a string, we converts `str` into `bytes`, which behave more like `std::string` as it is a sequence of bytes.  The following example tells more details.

```python
s = "Hi 益"
print(type(s))  # str
print(len(s))   # 4
print([ord(c) for c in s]) # [72, 105, 32, 30410]

b = s.encode('utf-8')
print(type(b))  # bytes
print(len(b))   # 6
print([i for i in b]) # [72, 105, 32, 231, 155, 138]
```
