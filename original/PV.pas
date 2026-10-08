unit PV;

interface
uses header;

function getPV(data:tdata;giliran,maxdepth:integer):string;
procedure printpv(var n:integer;spv:string;l:integer;t:longint;nilai3:integer);

implementation
uses hashing_header,hashing,notation,makemove,sysutils,winboard,windows;

function strtime(time:longint):string;
var st1,st2,st3:string;
begin
 st1:=inttostr(time div 3600);
 if strtoint(st1)<10 then st1:='0'+st1;
 st2:=inttostr((time mod 3600 )div 60);
 if strtoint(st2)<10 then st2:='0'+st2;
 st3:=inttostr(time mod 60);
 if strtoint(st3)<10 then st3:='0'+st3;
 result:=st1+':'+st2+':'+st3;
end;

procedure printpv(var n:integer;spv:string;l:integer;t:longint;nilai3:integer);
var s:string;
begin
    if not usewinboard then
    begin
{      if (n>3) then
        formiterative.StringGrid1.TopRow:=n-3
      else
        formiterative.StringGrid1.TopRow:=1;

      formiterative.stringgrid1.cells[3,n]:=spv;
      formiterative.stringgrid1.cells[0,n]:=inttostr(l);
  //    formiterative.stringgrid1.cells[1,n]:=strtime(floor((gettickcount-t)/ 1000));
      formiterative.stringgrid1.cells[1,n]:=strtime(t);
      formiterative.stringgrid1.cells[2,n]:=inttostr(nilai3);
      formiterative.StringGrid1.Repaint;
      inc(n);
      if n>formiterative.StringGrid1.RowCount then formiterative.StringGrid1.RowCount:=n;}
    end else
    if usewinboard then
    begin
       s:=inttostr(l)+' '+inttostr(nilai3)+' '+inttostr((gettickcount-ref2) div 10)+' '+inttostr(total_node)+' '+spv;;
       send_winboard(s);
//       send_winboard('tellall PV: '+spv+' Score : '+inttostr(nilai3)+' total node :'+inttostr(total_node));
    end;

end;

function getPV;
var nilai,hmoves,l:integer;
ab,pp,found:boolean;
a,b:integer;
begin
  hmoves:=_NO_MOVE;
  result:='';a:=_INFINITY+50000;b:=0;
  if not searchhash(data.hashkey,a,b,0,giliran,nilai,hmoves,found) then hmoves:=_NO_MOVE;
  l:=0;
//  if not found then hmoves:=_NO_MOVE;
  while hmoves<>_NO_MOVE do
  begin
    inc(l);
    result:=result+movetonotation(hmoves,data.ep,giliran);
    if l>=maxdepth+1 then exit;
    if giliran=_SISIHITAM then
      makeblackmove(hmoves,data)
    else
      makewhitemove(hmoves,data);
    giliran:=3-giliran;
    if not searchhash(data.hashkey,a,b,0,giliran,nilai,hmoves,found) then
      hmoves:=_NO_MOVE;
    result:=result+' ';
  end;

  
end;

end.
