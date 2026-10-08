unit undoredo;

interface
uses header;

type
tundomoves=record
  data:tdata;
  whiteclock,blackclock:integer;
end;

tundo=record
  count,max:integer;
  moves:array[1..400] of tundomoves;
end;

procedure resetundo(var undo:tundo);
procedure addundo(var undo:tundo;data:tdata;whiteclock,blackclock:integer);
function takeback(var undo:tundo;var data:tdata;var wc,bc:integer;var giliran:byte):boolean;
function moveforward(var undo:tundo;var data:tdata;var wc,bc:integer;var giliran:byte):boolean;

implementation

procedure resetundo(var undo:tundo);
begin
  undo.count:=0;undo.max:=0;
end;

procedure addundo;
begin
 with undo do
 begin
  if count=max then
  begin
    inc(count);
    inc(max);
    moves[count].data:=data;
    moves[count].whiteclock:=whiteclock;
    moves[count].blackclock:=blackclock;
  end else
  begin
    inc(count);
    max:=count;
    moves[count].data:=data;
    moves[count].whiteclock:=whiteclock;
    moves[count].blackclock:=blackclock;
  end;
 end;

end;

function moveforward(var undo:tundo;var data:tdata;var wc,bc:integer;var giliran:byte):boolean;
begin
 moveforward:=false;
 with undo do
 begin
    if count<max then
    begin
      moveforward:=true;
     { if count=1 then
        inc(count,0)
      else}
      if count=max-1 then
        inc(count,1)
      else
        inc(count,1);
      data:=moves[count].data;
      wc:=moves[count].whiteclock;
      bc:=moves[count].blackclock;
      giliran:=3-giliran;
    end;
 end;
end;

function takeback(var undo:tundo;var data:tdata;var wc,bc:integer;var giliran:byte):boolean;
begin
 takeback:=false;
 with undo do
 begin
   if count>0 then
    takeback:=true;
    if count=1 then
    begin
      dec(undo.count,0);
      giliran:=3-giliran;
    end else
{    if count=max then
      dec(undo.count,1)
    else}
      dec(count,1);
    giliran:=3-giliran;
    data:=moves[count].data;
    wc:=moves[count].whiteclock;
    bc:=moves[count].blackclock;
 end;

end;

end.
