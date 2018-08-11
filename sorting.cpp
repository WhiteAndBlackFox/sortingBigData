#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <memory>

using namespace std;

template< typename STREAM >
struct stream_t final {
    explicit stream_t( const string & file_name, ios::openmode mode )
        :   m_stream( file_name, mode )
    {
    }

    ~stream_t()
    {
        m_stream.close();
    }

    STREAM m_stream;
};


class sorted_file_t final {
public:
    explicit sorted_file_t( ifstream & stream )
        :   m_stream( stream )
        ,   m_cur( 0.0 )
    {
    }

    bool has_data()
    {
        if( !m_buf.empty() )
            return true;
        else
        {
            next();

            return ( !m_buf.empty() );
        }
    }

    double value() const
    {
        return m_cur;
    }

    void next()
    {
        m_buf.clear();

        getline( m_stream, m_buf );

        if( !m_buf.empty() )
        {
            istringstream str( m_buf );
            str >> m_cur;
        }
    }

private:
    ifstream & m_stream;
    string m_buf;
    double m_cur;
};

class sorted_splitted_data_t final {
public:
    explicit sorted_splitted_data_t( vector< sorted_file_t > & files )
        :   m_files( files )
    {
    }

    bool has_data()
    {
        return any_of( m_files.begin(), m_files.end(),
            [] ( sorted_file_t & file ) { return file.has_data(); } );
    }

    double min()
    {
        vector< pair< double, size_t > > values;

        size_t count = 0;

        for_each( m_files.begin(), m_files.end(),
            [ & ] ( sorted_file_t & file )
            {
                if( file.has_data() )
                    values.push_back( make_pair( file.value(), count ) );

                ++count;
            } );

        if( !values.empty() )
        {
            double min_value = values.front().first;
            size_t idx = values.front().second;

            for( const auto & p : values )
            {
                if( p.first < min_value )
                {
                    min_value = p.first;
                    idx = p.second;
                }
            }

            m_files[ idx ].next();

            return min_value;
        }
        else
            return 0.0;
    }

private:
    vector< sorted_file_t > & m_files;
};

ssize_t split_file( ifstream & stream )
{
    const size_t max_size = 1024 * 1024 * 100 / sizeof( double );

    ssize_t count = 0;

    while( stream )
    {
        vector< double > data;
        data.reserve( max_size );

        while( data.size() <= max_size && stream )
        {
            string text;
            getline( stream, text );

            if( !text.empty() )
            {
                istringstream str( text );
                double value = 0.0;
                str >> value;

                data.push_back( value );
            }
        }

        if( !data.empty() )
        {
            sort( data.begin(), data.end() );

            stream_t< ofstream > out( to_string( count ) + ".txt",
                ios::out | ios::trunc );

            if( out.m_stream )
            {
                for( const auto & v : data )
                    out.m_stream << v << "\n";
            }
            else
                return -1;

            ++count;
        }
    }

    return count;
}

int sortingfile()
{
    string fileData = "", resultData = "";
    cout << "Enter file for generate: ";
    cin >> fileData;

    cout << "Enter file for result: ";
    cin >> resultData;

    if (fileData != "" && resultData != ""){

        const auto start = chrono::high_resolution_clock::now();

        ssize_t count = 0;

        {
            stream_t< ifstream > in( "data.txt", ios::in );

            if( in.m_stream )
            {
                count = split_file( in.m_stream );

                if( count < 0 )
                {
                    cout << "Couldn't split input file\n";

                    return 1;
                }
            }
            else
            {
                cout << "Couldn't open input file.\n" ;

                return 1;
            }
        }

        vector< unique_ptr< stream_t< ifstream > > > in_streams;
        vector< sorted_file_t > in_files;

        for( ssize_t i = 0; i < count; ++i )
        {
            in_streams.push_back( make_unique< stream_t< ifstream > > (
                to_string( i ) + ".txt", ios::in ) );

            if( !in_streams.back()->m_stream )
            {
                cout << "Couldn't open splitted files.\n";

                return 1;
            }

            in_files.push_back( sorted_file_t( in_streams.back()->m_stream ) );
        }

        sorted_splitted_data_t data( in_files );
        stream_t< ofstream > out( "sorted_data.txt", ios::out | ios::trunc );

        if( !out.m_stream )
        {
            cout << "Couldn't open output file.\n";

            return 1;
        }

        while( data.has_data() )
            out.m_stream << data.min() << "\n";

        const auto finish = chrono::high_resolution_clock::now();

        auto duration = finish - start;

        const auto min = chrono::duration_cast< chrono::minutes > ( duration );

        duration -= min;

        const auto sec = chrono::duration_cast< chrono::seconds > ( duration );

        duration -= sec;

        const auto milli = chrono::duration_cast< chrono::milliseconds > ( duration );

        cout << "Elapsed time: "
            << min.count() << " m "
            << sec.count() << "."
            << setw( 3 ) << setfill( '0' ) << milli.count() << " s\n";
    }
    return 0;
}
