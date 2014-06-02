#include <boost/program_options.hpp>
#include "Osmosis/Server/Server.h"
#include "Osmosis/Client/CheckIn.h"
#include "Osmosis/Client/CheckOut.h"
#include "Osmosis/Client/ListLabels.h"
#include "Osmosis/Client/EraseLabel.h"

std::mutex globalTraceLock;

void server( const boost::program_options::variables_map & options )
{
	boost::filesystem::path rootPath( options[ "objectStoreRootPath" ].as< std::string >() );
	unsigned short port = options[ "serverTCPPort" ].as< unsigned short >();
	boost::asio::ip::tcp::endpoint endpoint( boost::asio::ip::tcp::v4(), port );

	Osmosis::ObjectStore::Store store( rootPath );
	Osmosis::ObjectStore::Drafts drafts( rootPath );
	Osmosis::ObjectStore::Labels labels( rootPath, store );
	Osmosis::Server::Server server( rootPath, endpoint, store, drafts, labels );
	server.run();
}

void checkIn( const boost::program_options::variables_map & options )
{
	boost::filesystem::path workDir = options[ "workDir" ].as< std::string >();
	std::string label = options[ "label" ].as< std::string >();
	std::string hostname = options[ "serverHostname" ].as< std::string >();
	unsigned short port = options[ "serverTCPPort" ].as< unsigned short >();
	bool md5 = options.count( "MD5" ) > 0;

	Osmosis::Client::CheckIn instance( workDir, label, hostname, port, md5 );
	instance.go();
}

void checkOut( const boost::program_options::variables_map & options )
{
	boost::filesystem::path workDir = options[ "workDir" ].as< std::string >();
	std::string label = options[ "label" ].as< std::string >();
	std::string hostname = options[ "serverHostname" ].as< std::string >();
	unsigned short port = options[ "serverTCPPort" ].as< unsigned short >();
	bool md5 = options.count( "MD5" ) > 0;
	bool removeUnknownFiles = options.count( "removeUnknownFiles" ) > 0;
	bool myUIDandGIDcheckout = options.count( "myUIDandGIDcheckout" ) > 0;

	Osmosis::FilesystemUtils::clearUMask();
	Osmosis::Client::CheckOut instance( workDir, label, hostname, port, md5, removeUnknownFiles, myUIDandGIDcheckout );
	instance.go();
}

void listLabels( const boost::program_options::variables_map & options )
{
	std::string labelRegex = ".*";
	if ( options.count( "workDir" ) > 0 )
		labelRegex = options[ "workDir" ].as< std::string >();
	boost::regex testExpression( labelRegex );
	std::string hostname = options[ "serverHostname" ].as< std::string >();
	unsigned short port = options[ "serverTCPPort" ].as< unsigned short >();

	Osmosis::Client::ListLabels instance( labelRegex, hostname, port );
	instance.go();
	for ( auto i = instance.result().begin(); i != instance.result().end(); ++ i )
		std::cout << * i << std::endl;
}

void eraseLabel( const boost::program_options::variables_map & options )
{
	std::string label = options[ "workDir" ].as< std::string >();
	std::string hostname = options[ "serverHostname" ].as< std::string >();
	unsigned short port = options[ "serverTCPPort" ].as< unsigned short >();

	Osmosis::Client::EraseLabel instance( label, hostname, port );
	instance.go();
}

void testHash( const boost::program_options::variables_map & options )
{
	boost::filesystem::path fileToHash = options[ "workDir" ].as< std::string >();
	bool md5 = options.count( "MD5" ) > 0;

	Osmosis::Hash hash = md5 ? Osmosis::CalculateHash::MD5( fileToHash ) : Osmosis::CalculateHash::SHA1( fileToHash );
	std::cout << hash << std::endl;
}

void usage( const boost::program_options::options_description & optionsDescription )
{
	std::cout << "osmosis.bin <command> [workDir] [label] [options]" << std::endl;
	std::cout << std::endl;
	std::cout << "  command:  can be 'server', 'checkin', 'checkout', 'listlabels'" << std::endl;
	std::cout << "            or 'eraselabel'" << std::endl;
	std::cout << "  workDir:  must be present if command is 'checkin' or 'checkout'" << std::endl;
	std::cout << "            workDir is the path to check in from or check out to" << std::endl;
	std::cout << "  label:    must be present if command is 'checkin', 'checkout' or" << std::endl;
	std::cout << "            'eraselabel'. label is the name of the dirlist to checkin" << std::endl;
	std::cout << "            or checkout." << std::endl;
	std::cout << "            optional for 'listlabels' command, in which case is " << std::endl;
	std::cout << "            treated as a regex" << std::endl;
	std::cout << "  testhash: just hash a localfile" << std::endl;
	std::cout << std::endl;
	std::cout << optionsDescription << std::endl;
}

int main( int argc, char * argv [] )
{
	boost::program_options::options_description optionsDescription( "options" );
	optionsDescription.add_options()
		("help", "produce help message")
		("objectStoreRootPath", boost::program_options::value< std::string >()->default_value( "/var/lib/osmosis/objectstore" ),
			"Path where osmosis will store objects. relevant for 'server' command, or if local object store is used" )
		("serverTCPPort", boost::program_options::value< unsigned short >()->default_value( 1010 ),
			"the TCP port to bind to, if command is 'server', or TCP port to connect to, if client")
		("serverHostname", boost::program_options::value< std::string >()->default_value( "localhost" ),
			"the hostname to connect to, if client" )
		( "MD5", "use MD5, not SHA1 for hash in 'checkin' operation" )
		( "removeUnknownFiles", "for checkout: remove files from disk that are not in the dirlist being checked out" )
		( "myUIDandGIDcheckout", "for checkout: use my uid and gid" );

	boost::program_options::options_description positionalDescription( "positionals" );
	positionalDescription.add_options()
		( "command", boost::program_options::value< std::string >() )
		( "workDir", boost::program_options::value< std::string >() )
		( "label", boost::program_options::value< std::string >() );

	boost::program_options::positional_options_description positionalMapping;
	positionalMapping.add( "command", 1 ).add( "workDir", 1 ).add( "label", 1 );

	boost::program_options::options_description allOptions;
	allOptions.add( optionsDescription ).add( positionalDescription );

	boost::program_options::variables_map options;
	try {
		boost::program_options::store(
			boost::program_options::command_line_parser( argc, argv ).
				positional( positionalMapping ).options( allOptions ).
				run(),
			options );
		boost::program_options::notify( options );
	} catch ( boost::exception & e ) {
		TRACE_BOOST_EXCEPTION( e, "Unable to parse command line" );
		usage( optionsDescription );
		return 1;
	}

	if ( options.count( "help" ) ) {
		usage( optionsDescription );
		return 1;
	}

	try {
		std::string command = options[ "command" ].as< std::string >();
		if ( command == "server" ) {
			if ( options.count( "workDir" ) > 0 or options.count( "label" ) > 0 ) {
				TRACE_ERROR( "'workDir' or 'label' must not be present in command line"
						"if 'server' is specified as the command" );
				usage( optionsDescription );
				return 1;
			}
			server( options );
		} else if ( command == "checkin" )
			checkIn( options );
		else if ( command == "checkout" )
			checkOut( options );
		else if ( command == "listlabels" )
			listLabels( options );
		else if ( command == "eraselabel" )
			eraseLabel( options );
		else if ( command == "testhash" )
			testHash( options );
		else {
			TRACE_ERROR( "Unknown command '" << command << "'" );
			usage( optionsDescription );
			return 1;
		}
	} catch ( boost::exception & e ) {
		TRACE_BOOST_EXCEPTION( e, "Terminated on a boost exception" );
		return 1;
	} catch ( Error & e ) {
		TRACE_ERROR( "Terminated on 'Error' exception: '" << e.what() << "' at " << e.filename << ':' << e.line );
		return 1;
	} catch ( std::exception & e ) {
		TRACE_ERROR( "Terminated on std::exception: '" << e.what() );
		return 1;
	} catch ( ... ) {
		TRACE_ERROR( "Terminated on unknown exception" );
		return 1;
	}

	return 0;
}
