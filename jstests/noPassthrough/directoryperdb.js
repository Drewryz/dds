var baseDir = "jstests_directoryperdb";
port = allocatePorts( 1 )[ 0 ];
dbpath = MongoRunner.dataPath + baseDir + "/";

var m = MongoRunner.runMongod({
    dbpath: dbpath,
    port: port,
    directoryperdb: ''});
db = m.getDB( "foo" );
db.bar.insert( { x : 1 } );
assert.eq( 1, db.bar.count() );

db.adminCommand( {fsync:1} );
files = listFiles( dbpath ).filter( function(z) {
    return z.name.endsWith( "/foo" );
} );
assert.eq( 1, files.length );

files = listFiles( files[0].name );
assert( files.length > 0 );

MongoRunner.stopMongod(port);

// Subsequent attempt to start server using same dbpath without directoryperdb should fail.
assert.isnull(MongoRunner.runMongod({
    dbpath: dbpath,
    port: port,
    restart: true}));
