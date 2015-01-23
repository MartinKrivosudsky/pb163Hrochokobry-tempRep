
#include <string>
#include <iostream>
#include <sstream>

#include "Console.h"
#include "ClientThread.h"
#include "Server.h"


Console::Console( QObject* parent )
:   QThread( parent )
{
    m_window = new QWidget;
    m_window->setMinimumWidth( 50 );
    m_window->setMinimumHeight( 50 );
    m_window->setGeometry( 400, 300, 400, 300 );
    m_window->setFixedSize( 400, 300 );
    m_textBox = new QTextEdit( m_window );
    m_textBox->setGeometry( 0, 0, 400, 280 );
    m_textBox->setReadOnly( true );
    m_line = new QLineEdit( m_window );
    m_line->setGeometry( 10, 280, 390, 20 );
    connect( m_line, SIGNAL( returnPressed() ), this, SLOT( execute() ) );
    m_label = new QLabel( m_window );
    m_label->setGeometry( 0, 280, 10, 20 );
    m_label->setText( ">" );
    m_window->show();
}

void Console::run()
{
    exec();
}

void Console::execute()
{
    switch( m_command )
    {
    case NONE:
        if( !m_line->text().compare( "db" ) )
        {
            m_command = DB;
        }
        else if( !m_line->text().compare( "msg" ) )
        {
            m_command = MESSAGE;
        }
        else
        {
            m_textBox->append( "Invalid command." );
        }
        break;
    case DB:
        {
            QSqlQuery query( Server::s_db );

            if( query.exec( m_line->text() ) )
                m_textBox->append( "Query executed" );
            else
            {
                m_textBox->append( query.lastError().text() );
                m_command = NONE;
                break;
            }

            std::stringstream result;
            while( query.next() )
            {
                QSqlRecord record = query.record();
                for( int i = 0; i < record.count(); i++ )
                {
                    QSqlField field = record.field( i );
                    QVariant variant = field.value();
                    result << " | " << field.name().toStdString() << ": "
                           << variant.toString().toStdString();
                }
                result << std::endl << "-----" << std::endl;
            }
            m_textBox->append( result.str().c_str() );

        }
        m_command = NONE;
        break;
    case MESSAGE:
        {
            std::stringstream ss( m_line->text().toStdString() );
            std::string name, msg;
            ss >> name >> msg;
            std::map<std::string, ClientThread*>::iterator i = Server::s_threadList.find( name );
            if( i != Server::s_threadList.end() )
            {
                i->second->SendMsg( msg );
            }
            else
            {
                m_textBox->append( std::string( "User " + name + " not found." ).c_str() );
            }
        }
        m_command = NONE;
        break;
    default:
        m_textBox->append( "Invalid command." );
        m_command = NONE;
        break;
    }
    m_line->setText( "" );
}
