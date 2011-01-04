doIt = false;
dbpath = "/data/db/diskfulltest";

files = listFiles( "/data/db" );
for ( i in files ) {
    if ( files[ i ].name == dbpath ) {
        doIt = true;
    }
}

if ( !doIt ) {
    print( "path " + dbpath + " missing, skipping diskfull test" );
    doIt = false;
}

// Clear dbpath without removing and recreating diskfulltest directory, as resetDbpath does
files = listFiles( dbpath );
files.forEach( function( x ) { removeFile( x.name ) } );

if ( doIt ) {
    port = allocatePorts( 1 )[ 0 ];
    m = startMongoProgram( "mongod", "--port", port, "--dbpath", dbpath, "--nohttpinterface", "--bind_ip", "127.0.0.1" );
    c = m.getDB( "diskfulltest" ).getCollection( "diskfulltest" )
    c.save( { a: 6 } );
    assert.soon( function() { return rawMongoProgramOutput().match( /file allocation failure/ ); }, "didn't see 'file allocation failure'" );
    assert.isnull( c.findOne() , "shouldn't exist" );
    sleep( 3000 );
    m2 = new Mongo( m.host );
    printjson( m2.getDBs() );
}
