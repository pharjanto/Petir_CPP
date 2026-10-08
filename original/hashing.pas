unit hashing;


interface
uses header,hashing_header;


PROCEDURE addtable(key:int64;giliran,level,moves,nilai,tipe:integer);
function searchhash(key:int64;var alpha,beta:integer;level,giliran:Integer;VAR nilai:integer;VAR moves:Integer;var avoidNM:Boolean):boolean;

implementation
uses valid;


PROCEDURE addtable;
VAR hkey:integer;
kk:^thashrecord;
BEGIN
    //menentukan index hash table
    if giliran=_SISIHITAM then key:=not key;
    hkey:=key AND (maxhash-1);
    level:=level shr 1;
    kk:=@hashtable[hkey];
    IF (kk^.flag and bit_used=0) THEN
    BEGIN
        //jika index hash masih kosong, maka simpan
        //ke entry tersebut
        kk^.flag:=kk^.flag or bit_used;

        kk^.key:=key;
        kk^.nilai:=nilai;
        kk^.tipe:=tipe;
        kk^.level:=level;
        kk^.moves:=moves;
        kk^.flag:=kk^.flag and not bit_old;
    END ELSE
    BEGIN
            IF (kk^.flag and bit_old <>0) or (level>=kk^.level)  then
            BEGIN
              kk^.moves:=moves;
              kk^.key:=key;
              kk^.nilai:=nilai;
              kk^.tipe:=tipe;
              kk^.level:=level;
              kk^.flag:=kk^.flag and not bit_old;
            END;
    END;
END;

var anu:integer=0;

function searchhash;
VAR hkey:integer;
temp:^thashrecord;
BEGIN
//    exit;
    if giliran=_SISIHITAM then key:=not key;
    hkey:=key AND (MAXHASH-1);
    temp:=@hashtable[hkey];
    avoidnm:=false;
    IF (temp^.flag and bit_used <>0) THEN
    BEGIN
        IF (temp^.key=key)
        //and ((temp^.moves=0) or isvalid(data,temp^.moves,giliran))
        THEN
        BEGIN
{            inc(anu);
            if not isvalid(data,temp^.moves,giliran) then
            begin
              inc(anu);
            end;  }
            //jika key ditemukan
            //catat langkah terbaik yang disimpan pada TT
            moves:=temp^.moves;
            {$IFDEF stat}
            inc(hash_hit);
            {$ENDIF stat}
//            if temp^.level>=start_nm_level then no_null:=true;
            IF (temp^.level>=level shr 1) THEN
            BEGIN //syarat agar bisa digunakan adalah level pada TT > level
              case temp^.tipe of
                PV_NODE :
                  begin
                    nilai:=temp^.nilai;result:=true;exit;
                  end;
                FAIL_LOW :
                  if temp^.nilai<=alpha then
                  begin
                    nilai:=temp^.nilai;result:=true;exit;
                  end else
                  if temp^.nilai>=beta then
                  begin
                    if (temp^.level*2>=16) then
                      avoidNM:=true;
                  end;
                FAIL_HIGH :
                  if temp^.nilai>=beta then
                  begin
                    nilai:=temp^.nilai;result:=true;exit;
                  end;
              end;
            END;
        END;
    END;
    result:=false;
END;

end.
