#include "PreProcessor.h"

#include "Logger/Logger.h"
#include "rhi_Utils.h"

PreProc::DefFileCallback    PreProc::_DefFileCallback;


//------------------------------------------------------------------------------

PreProc::PreProc(FileCallback* fc)
  : _file_cb((fc)?fc:&_DefFileCallback)
{
}


//------------------------------------------------------------------------------

PreProc::~PreProc()
{
    clear();
}


//------------------------------------------------------------------------------

bool    
PreProc::process_file( const char* file_name, TextBuf* output )
{
    bool    success = false;

    if( _file_cb->open( file_name ) )
    {
        unsigned    text_sz = _file_cb->size();
        char*       text    = _alloc_buffer( text_sz+1 );

        _file_cb->read( text_sz, text );
        text[text_sz] = 0;
        _file_cb->close();

        if( _process_buffer( text, &_line ) )
        {
            _generate_output( output );
            success = true;
        }
    }

    return success;
}


//------------------------------------------------------------------------------

bool
PreProc::process_inplace( char* src_text, TextBuf* output )
{
    bool    success = false;

    _reset();
    if( _process_buffer( src_text, &_line ) )
    {
        _generate_output( output );
        success = true;
    }

    return success;
}


//------------------------------------------------------------------------------

bool    
PreProc::process( const char* src_text, TextBuf* output )
{
    bool    success = false;
    char*   text    = _alloc_buffer( strlen(src_text) );

    strcpy( text, src_text );

    if( process_inplace( text, output ) )
    {
        _generate_output( output );
        success = true;
    }

    return success;
}


//------------------------------------------------------------------------------

void    
PreProc::clear()
{
    _line.clear();
    
    for( unsigned b=0; b!=_buf.size(); ++b )
        ::free( _buf[b].mem );
    _buf.clear();
}


//------------------------------------------------------------------------------

void    
PreProc::dump() const
{
    for( unsigned i=0; i!=_line.size(); ++i )
    {
        DAVA::Logger::Info( "%04u | %s", _line[i].line_n, _line[i].text );
    }
}


//------------------------------------------------------------------------------

void
PreProc::_reset()
{
    clear();
}


//------------------------------------------------------------------------------

char*   
PreProc::_alloc_buffer( unsigned sz )
{
    Buffer  buf;

    buf.mem = ::malloc( sz );

//    _buf.append( 1, &buf, 16 );
    _buf.push_back( buf );
    return (char*)(buf.mem);
}


//------------------------------------------------------------------------------

bool    
PreProc::_process_buffer( char* text, std::vector<Line>* line )
{
    bool        success = true;
    char*       ln      = text;    
    unsigned    line_n  = 1;

    int         condition     = 1;
    int         pending_endif = 0;
    int skip_lines = false;

    for( char* s=text; *s; ++s ) 
    {
        if( *s == '\r' )
        {
            if( s[1] == '\n' )
                *s++ = 0;
            else
                *s = ' ';
        }

        if( skip_lines )
        {
            // ignore entire line

            while( *s != '\n' )
                ++s;
            if( *s == 0 )
                break;
            else
                ln = s = s+1;
        }

        if( *s == '\n' )
        {
            *s = 0;
            line->push_back( Line(ln,line_n) );
            ln = s+1;
            ++line_n;
        }
        else if( *s == '#' )
        {
            DVASSERT(s[1]);
            if( strncmp( s+1, "include", 7 ) == 0 )
            {
                char*   t  = s;
                char*   f0 = nullptr;
                char*   f1 = nullptr;
                
                while( *t != '\"' )
                    ++t;
                DVASSERT(*t);
                f0 = t+1;
                ++t;
                while( *t != '\"' )
                    ++t;
                DVASSERT(*t);
                f1 = t-1;

                char    fname[256];

                strncpy( fname, f0, f1-f0+1 );
                fname[f1-f0+1] = 0;
                _process_include( fname, line );

                while( *t != '\n' )
                    ++t;
                if( *t == 0 )
                {
                    break;
                }
                else
                {
                    ln = s = t+1;
                }
            }
            else if( strncmp( s+1, "define", 6 ) == 0 )
            {
                char*   t  = s+1+6;
                char*   n0 = nullptr;
                char*   n1 = nullptr;
                char*   v0 = nullptr;
                char*   v1 = nullptr;
                char    name[128];
                char    val[16];

                while( *t == ' '  ||  *t == '\t'  )
                    ++t;
                DVASSERT(*t);
                n0 = t;
                while( *t != ' '  &&  *t != '\t'  )
                    ++t;
                DVASSERT(*t);
                n1 = t-1;


                while( *t == ' '  ||  *t == '\t'  )
                    ++t;
                DVASSERT(*t);
                v0 = t;
                while( *t != ' '  &&  *t != '\t'  &&  *t != '\n'  )
                    ++t;
                DVASSERT(*t);
                v1 = t-1;

                strncpy( name, n0, n1-n0+1 );
                name[n1-n0+1] = 0;

                strncpy( val, v0, v1-v0+1 );
                val[v1-v0+1] = 0;

                _process_define( name, val );
                while( *t  &&  *t != '\n' )
                    ++t;
                if( *t == 0 )
                {
                    break;
                }
                else
                {
                    s  = t;
                    ln = t + 1;
                }
            }
            else if( strncmp( s+1, "ifdef", 5 ) == 0 )
            {
                char*   t  = s+1+5;
                char*   n0 = nullptr;
                char*   n1 = nullptr;
                char    name[128];

                while( *t == ' '  ||  *t == '\t'  )
                    ++t;
                DVASSERT(*t);
                n0 = t;
                while( *t != ' '  &&  *t != '\t'  &&  *t != '\r'  &&  *t != '\n'  )
                    ++t;
                DVASSERT(*t);
                n1 = t-1;

                strncpy( name, n0, n1-n0+1 );
                name[n1-n0+1] = 0;
                
                while( *s != '\n' )
                    ++s;
                
                ln = s = s+1;
                condition  = (_eval.has_variable( name ))  ? 1  : 0;
                skip_lines = !condition;
                ++pending_endif;
            }
            else if( strncmp( s+1, "ifndef", 6 ) == 0 )
            {
                char*   t  = s+1+6;
                char*   n0 = nullptr;
                char*   n1 = nullptr;
                char    name[128];

                while( *t == ' '  ||  *t == '\t'  )
                    ++t;
                DVASSERT(*t);
                n0 = t;
                while( *t != ' '  &&  *t != '\t'  &&  *t != '\r'  &&  *t != '\n'  )
                    ++t;
                DVASSERT(*t);
                n1 = t-1;

                strncpy( name, n0, n1-n0+1 );
                name[n1-n0+1] = 0;
                
                while( *s != '\n' )
                    ++s;
                
                ln = s = s+1;
                condition  = (_eval.has_variable( name ))  ? 0  : 1;
                skip_lines = !condition;
                ++pending_endif;
            }
            else if( strncmp( s+1, "if", 2 ) == 0 )
            {
                char*   e = s+1 + 2;
                
                while( *s != '\n' )
                    ++s;
                DVASSERT(*s);
                *s = 0;


                float   v=0;
                if(!_eval.evaluate( e, &v ))
                {
                    char    err[256];

                    _eval.get_last_error( err, countof(err) );
DAVA::Logger::Error( err );
//DUMP((char*)err);
                }

                ln = s = s+1;
                condition  = int(v);
                skip_lines = !condition;
                ++pending_endif;
            }
            else if( strncmp( s+1, "elif", 4 ) == 0 )
            {
                char*   e = s+1 + 4;
                
                while( *s != '\n' )
                    ++s;
                DVASSERT(*s);
                *s = 0;

                float   v=0;

                if(!_eval.evaluate( e, &v ))
                {
                    char    err[256];

                    _eval.get_last_error( err, countof(err) );
DAVA::Logger::Error( err );
//DUMP((char*)err);
                }

                ln = s = s+1;
                condition  = int(v);
                skip_lines = !condition;
            }
            else if( strncmp( s+1, "else", 4 ) == 0 )
            {
                skip_lines = condition;                    

                while( *s != '\n' )
                    ++s;
                if( *s == 0 )
                    break;
                else
                    ln = s+1;
            }
            else if( strncmp( s+1, "endif", 5 ) == 0 )
            {
                --pending_endif;
                skip_lines = false;

                while( *s != '\n' )
                    ++s;
                if( *s == 0 )
                    break;
                else
                    ln = s+1;
            }
        }
    }

    if( ln[0] )
        line->push_back( Line(ln,line_n) );

    return success;
}


//------------------------------------------------------------------------------

bool    
PreProc::_process_include( const char* file_name, std::vector<PreProc::Line>* line )
{   
    bool    success = true;

    if( _file_cb->open( file_name ) )
    {
        unsigned    text_sz = _file_cb->size();
        char*       text    = _alloc_buffer( text_sz+1 );

        _file_cb->read( text_sz, text );
        text[text_sz] = 0;
        _file_cb->close();

        _process_buffer( text, &_line );
    }


    return success;
}


//------------------------------------------------------------------------------

bool
PreProc::_process_define( const char* name, const char* value )
{
    bool    success = false;
    float   val;

    if( _eval.evaluate( value, &val ) )
    {
        _var.push_back( Var() );
        strcpy( _var.back().name, name );
        _var.back().val = int(val);
    
        _eval.set_variable( name, val );
    }

    return success;
}


//------------------------------------------------------------------------------

void    
PreProc::_generate_output( TextBuf* output )
{
    static const char*  endl = "\r\n";

    output->clear();
    for( std::vector<Line>::const_iterator l=_line.begin(),l_end=_line.end(); l!=l_end; ++l )
    {
        unsigned    sz = strlen( l->text );

        output->insert( output->end(), l->text, l->text+sz );
        output->insert( output->end(), endl, endl+2 );
    }
}
