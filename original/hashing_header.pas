unit hashing_header;

interface
uses header;

const

  bit_used=64;
  bit_old=128;

type
  thashrecord=packed record
        flag,tipe:byte;
        key:int64;
        level:byte;
        moves:word;
        nilai:smallint;
        mte:boolean;
  END;

  thashdata=array of thashrecord;

PROCEDURE inithashkey;
FUNCTION hashvalue(VAR data:tdata;giliran:integer):int64;
PROCEDURE cleartable;
PROCEDURE cleartable2;
procedure inittable;
PROCEDURE clearreversetable;
PROCEDURE reversetable;

var
  hashtable:thashdata;
  anu:integer;

implementation

procedure inittable;
begin
// hashtable:=nil;
 setlength(hashtable,maxhash);

end;

PROCEDURE cleartable;
VAR a:integer;
BEGIN

  FOR a:=0 TO maxhash-1 DO
  BEGIN
        hashtable[a].flag:=hashtable[a].flag and not bit_used;
        hashtable[a].flag:=hashtable[a].flag and not bit_old;
  END;
END;

PROCEDURE reversetable;
BEGIN
END;

PROCEDURE clearreversetable;
BEGIN
END;


PROCEDURE cleartable2;
VAR a:integer;
BEGIN

  FOR a:=0 TO maxhash-1 DO
  BEGIN
        hashtable[a].flag:=hashtable[a].flag or bit_old;
  END;
END;

FUNCTION hashvalue;
VAR t:int64;
a:integer;
BEGIN
  t:=0;
  for a:=0 to 63 do
    if data.papan[a]<>0 then
      t:=t xor hashkey[data.papan[a],a];
  t:=t xor rokadehashkey[data.flagrokade];
  t:=t xor enpassanthashkey[data.ep];
//  t:=t or (giliran-1);
  hashvalue:=t;
END;


PROCEDURE inithashkey;
VAR a,b:integer;
BEGIN
  randseed:=99999; //lucky number ^_^
  for a:=-6 TO 6 DO
  begin
    for b:=0 TO 127 DO
      if b<=63 then
        hashkey[a,b]:=(int64(random(214748367)+1)*int64(random(214748367)+1)*(int64(random(65536)+8192)) shl 1)
      else
        anu:=int64((random(214748367)+1)*int64(random(214748367)+1)*(int64(random(65536)+8192)) shl 1);
  end;
  for a:=0 TO 255 DO
  begin
      rokadehashkey[a]:=(int64(random(214748367)+1)*int64(random(214748367)+1)*(int64(random(65536)+8192)) shl 1);
  end;
  for a:=0 TO _NO_EP DO
  begin
      enpassanthashkey[a]:=(int64(random(214748367)+1)*int64(random(214748367)+1)*(int64(random(65536)+8192)) shl 1);
  end;
  ephashkey:=enpassanthashkey[10];

END;

procedure coba;
begin
end;



end.
