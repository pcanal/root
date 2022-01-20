\page core %Accelogic lossy compression

\tableofcontents

## BLAST

Accelogic is providing a set of lossy compression algorithm in the BLAST library.

To select this compression engine use: `ROOT::EAlgorithm::kBLAST`

For float and double, the range of compression 'level' is 1 through 71 inclusive.

For integer type (as the moment) there is only one level of compression.

## Supported data types

ROOT current supports the compression of the following data types through
the Accelogic algorithms:

- float
- double
- signed and unsigned short
- signed and unsigned int
- signed and unsigned long long

and 

- fixed size array of the above listed types

The following are currently explicitly not supported:

- signed and unsigned char
- signed and unsigned long
- Double32_t (stored as float)
- variable size array of numerical types
- STL collection of numerical types

The following can not be supported:

- boolean
- Double32_t (stored as packed ints)
- Float16_t
- strings (`const char*`, `TString`, `std::string`)
- Any aggregate types

** Currently if the Accelogic compression is requested for a non-supported type,
it is stored uncompressed **