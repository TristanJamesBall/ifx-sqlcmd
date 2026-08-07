# Sportsbet SQLCMD

This is a patched version of the Informix User Group sqlcmd, with a few local fixes:

* Fixups to the manpage
* Improved csv output ( "excel compatible" )
* bigint and related types are treated as numbers for csv output ( that is, they're unquoted )
* Improved html table output

# Build Notes

( Note this isn't fully pipeline automated yet, and might never be, it changes very, very rarely ) 

While we're supporting both Linux2 and Linux2023 we need two versions of this ( and static linking isn't an option due to glibc dependencies )

What that means in practice is that this needs to be build twice, once on each platform

A pre-existing SB Informix environment ( including Informix SDK  ) is assumed.

You can run this on the primary node in the dev/tst environment for example.

## Build steps

```
git clone https://github.com/Sportsbet-Internal/dba-sqlcmd.git
cd dba-sqlcmd
./00_build_prereqs.sh  
./01_configure.sh  
./02_build.sh  
./03_install_and_package.sh
```

This will create either:
```
sqlcmd.amzn2023.tgz
```
or

```
sqlcmd.amzn2.tgz
```
Which then needs to be manually copied to:
```
s3://sb-openbet-builds-i6w0a40xjrstbble/software/
```

For a _very_ basic test of the patched functionality:
```
./99_test_sqlcmd.sh


Query is:

        select
          5::bigint                             as bigint,
          'hello'                               as string,
          'string with " in it'                 as tricky_string
          ,'string with'||chr(10)||' newline'   as newline_String
        from
          sysmaster:sysdual;



Output should look like:

        "bigint"  "string"  "tricky_string"         "newline_string"
        5         "hello"   "string with "" in it"  "string with
        newline"


Runing test:

        "bigint"  "string"  "tricky_string"         "newline_string"
        5         "hello"   "string with "" in it"  "string with
        newline"

```
