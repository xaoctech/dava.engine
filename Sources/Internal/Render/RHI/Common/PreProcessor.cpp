#include "PreProcessor.h"

#include "Logger/Logger.h"
#include "rhi_Utils.h"

PreProc::DefFileCallback    PreProc::_DefFileCallback;


//------------------------------------------------------------------------------

PreProc::PreProc(FileCallback* fc)
  : _file_cb((fc)?fc:&_DefFileCallback),
    _cur_file_name("<buffer>")
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

        _cur_file_name = "<buffer>";
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
    _reset();
    return _process_inplace( src_text, output );
}


//------------------------------------------------------------------------------

bool    
PreProc::process( const char* src_text, TextBuf* output )
{
    _reset();

    bool    success = false;
    char*   text    = _alloc_buffer( unsigned(strlen(src_text))+1 );

    strcpy( text, src_text );

    if( _process_inplace( text, output ) )
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

bool    
PreProc::add_define( const char* name, const char* value )
{
    return _process_define( name, value );
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
    _cur_file_name = "<buffer>";
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
    bool        success       = true;
    char*       ln            = text;    
    unsigned    line_n        = 1;
    unsigned    src_line_n    = 1;

    int                 skip_lines      = false;
    bool                dcheck_pending  = true;
    std::vector<int>    pending_endif;

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
            
            ++src_line_n;
            dcheck_pending = true;
        }

        if( *s == '\n' )
        {
            *s = 0;
            line->push_back( Line(ln,line_n) );
            ln = s+1;
            ++line_n;
            ++src_line_n;
            dcheck_pending = true;
        }
        else if( dcheck_pending )
        {
            char* ns1 = s;

            while( *ns1 == ' '  ||  *ns1 == '\t' )
                ++ns1;

            if( *ns1 == '#' )
            {
                s = ns1;

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
                
                    ln = s+1;
                    int condition = (_eval.has_variable( name ))  ? 1  : 0;
                    pending_endif.push_back( condition );

                    skip_lines = !condition;
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
                
                    ln = s+1;

                    int condition = (_eval.has_variable( name ))  ? 0  : 1;
                    skip_lines = !condition;
                    pending_endif.push_back( condition );
                }
                else if( strncmp( s+1, "if", 2 ) == 0 )
                {
                    char*   e = s+1 + 2;
                
                    while( *s != '\n' )
                        ++s;
                    DVASSERT(*s);
                    *s = 0;


                    float   v=0;
                    if( !_eval.evaluate( e, &v ) )
                    {
                        _report_expr_eval_error( src_line_n );
                    }

                    ln = s+1;

                    int condition = int(v);
                    skip_lines = !condition;
                    pending_endif.push_back( condition );
                }
                else if( strncmp( s+1, "elif", 4 ) == 0 )
                {
                    char*   e = s+1 + 4;
                
                    while( *s != '\n' )
                        ++s;
                    DVASSERT(*s);
                    *s = 0;

                    float   v=0;

                    if( !_eval.evaluate( e, &v ) )
                    {
                        _report_expr_eval_error( src_line_n );
                    }

                    ln = s+1;

                    int condition = int(v);
                    skip_lines = !condition;
                    pending_endif.back() =  condition;
                }
                else if( strncmp( s+1, "else", 4 ) == 0 )
                {
                    skip_lines = pending_endif.back();

                    while( *s != '\n' )
                        ++s;
                    if( *s == 0 )
                        break;
                    else
                        ln = s+1;
                }
                else if( strncmp( s+1, "endif", 5 ) == 0 )
                {
                    pending_endif.pop_back();
                    skip_lines = (pending_endif.size())  ? !pending_endif.back()  : false;

                    while( *s != '\n' )
                        ++s;
                    if( *s == 0 )
                        break;
                    else
                        ln = s+1;
                }
                
                dcheck_pending = true;
            }
            else
            {
                if( *ns1 == '\n' )
                    s = ns1-1;                
                
                dcheck_pending = false;
            }            
        }
    }

    if( ln[0] )
        line->push_back( Line(ln,line_n) );

    return success;
}


//------------------------------------------------------------------------------

bool    
PreProc::_process_inplace( char* src_text, TextBuf* output )
{
    bool    success = false;

    if( _process_buffer( src_text, &_line ) )
    {
        _generate_output( output );
        success = true;
    }

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

        const char* prev_file_name = _cur_file_name;

        _cur_file_name = file_name;
        _process_buffer( text, &_line );
        _cur_file_name = prev_file_name;
    }
    else
    {
        DAVA::Logger::Error( "failed to open \"%s\"\n", file_name );
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
        unsigned    sz = unsigned(strlen( l->text ));

        output->insert( output->end(), l->text, l->text+sz );
        output->insert( output->end(), endl, endl+2 );
    }
}


//------------------------------------------------------------------------------

void    
PreProc::_report_expr_eval_error( unsigned line_n )
{
    char    err[256];

    _eval.get_last_error( err, countof(err) );
    DAVA::Logger::Error( "%s  : %u  %s", _cur_file_name, line_n, err );
}
