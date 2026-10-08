unit repetition;

interface
const
    maxhashdraw=32768*2;
type
    treprecord=packed record
      used:boolean;
      key:int64;
      jumlah:shortint;
    end;
 tarrreprecord=array[0..1,0..maxhashdraw-1] of treprecord;
var
  repdata:tarrreprecord;
procedure initrepdata;
function addrephash(key:int64;giliran:integer):integer;
procedure delrephash(key:int64;giliran:integer);

implementation
uses header;

procedure initrepdata;
var a,b:integer;
begin
  for b:=0 to 1 do
  for a:=0 to maxhashdraw-1 do
  begin
    repdata[b,a].used:=false;
    repdata[b,a].jumlah:=0;
  end;

end;

function isrep(key:int64;giliran:integer):boolean;
var hkey:int64;
begin
  hkey:=key AND (maxhashdraw-1);
//  anu4:=giliran;
//  inc(teet);
  while (repdata[giliran,hkey].key<>key) do
  begin
    if hkey=maxhashdraw-1 then
      hkey:=-1;
    inc(hkey);
  end;
  if repdata[giliran,hkey].jumlah=2 then
  begin
    result:=true;exit;
  end;
  result:=false;
end;

function addrephash;
var hkey:int64;
p:^treprecord;
begin

  hkey:=key AND (maxhashdraw-1);
  addrephash:=-1;
//  inc(teet);
  while (repdata[giliran,hkey].used) and (repdata[giliran,hkey].key<>key) do
  begin
    if hkey=maxhashdraw-1 then
      hkey:=-1;
    inc(hkey);
  end;
  p:=@repdata[giliran,hkey];
  if p^.used=false then
  begin
    p^.used:=true;
    p^.key:=key;
    p^.jumlah:=0;
    result:=0;
  end else
  begin
    inc(p^.jumlah);
    result:=p^.jumlah;
  end;
end;

procedure delrephash;
var hkey:int64;
begin

  hkey:=key AND (maxhashdraw-1);
//  anu4:=giliran;
//  inc(teet);
  while (repdata[giliran,hkey].key<>key) do
  begin
    if hkey=maxhashdraw-1 then
      hkey:=-1;
    inc(hkey);
  end;
  if repdata[giliran,hkey].jumlah=0 then
  begin
    repdata[giliran,hkey].used:=false;
  end else
  dec(repdata[giliran,hkey].jumlah);
end;

end.
