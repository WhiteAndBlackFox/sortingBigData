#include <iostream>
#include <fstream>
#include <sstream>
#include <random>

using namespace std;

template< typename STREAM >
struct stream_t final {
    explicit stream_t( const string & file_name )
        :   m_stream( file_name )
    {
    }

    ~stream_t()
    {
        m_stream.close();
    }

    STREAM m_stream;
};

int gen()
{
    string genFile = "";
    cout << "Enter file name: ";
    cin >> genFile;
    if (genFile != ""){
        stream_t< ofstream > stream(genFile);

        random_device rd;
        mt19937 gen( rd() );
        uniform_real_distribution< double > dis( -9999999.0, 9999999.0 );

        fstream::pos_type size = 0;

        const auto max_file_size = 1024 * 1024 * 1024 * 1;

        while( size < max_file_size )
        {
            ostringstream str;
            str << dis( gen );

            const auto text = str.str();

            stream.m_stream << text << "\n";

            size += ( text.length() + 1 );

        }
    }
    cout << "press any key to exit";

    return 0;
}
