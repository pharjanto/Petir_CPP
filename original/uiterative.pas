unit uiterative;

interface
uses header;
function iterative(level,giliran,tipe:integer;data:tdata):integer;

implementation
uses search,math,root,pv,notation,windows,tools,movgen;
const
  timereducement:array[2..12] of integer=
  (1,5,15,20,20,25,30,30,30,30,30);

var retime:integer;
FUNCTION mtdf(data:tdata;level,giliran:byte;firstguess:integer):integer;
VAR oldg:integer;
beta,g:integer;
momentum:real;
spv:string;
oldmoves,globalupper,globallower:integer;
up,down:boolean;
dummy:byte;

BEGIN
    momentum:=1.5;
    g:=firstguess;
    globalupper:=_infinity+100;
    globallower:=-_infinity-100;
//    mtdit:=0;
    beta:=g;
//    mtditr:=0;
    oldmoves:=0;

    up:=false;down:=false;

    repeat
        oldg:=g;
        g:=searchroot(level,giliran,_NODEMAX,beta-1,beta,data,false,0,round(maxext*level));

        IF g<beta THEN
        BEGIN
         globalupper:=g;
         beta:=g;
         if not up then
           beta:=beta+round(momentum*(g-oldg));
         down:=true;

        END ELSE
        BEGIN
          globallower:=g;
          beta:=g+1;

          IF not down THEN
            beta:=beta+round(momentum*(g-oldg));
          if mmoves<>oldmoves then
          begin
            spv:=getpv(data,giliran,(level div fractional_ply)+3);
            printpv(barisitr,spv,level div fractional_ply,floor((gettickcount-startclock)/ 1000),g);
          end;
          up:=true;
          oldmoves:=mmoves;
        END;
    until globallower>=globalupper;
    mmoves:=oldmoves;
    mtdf:=g;
END;


function aspiration(level,giliran,tipe,nilai:integer;data:tdata):integer;
var n,n2,d,d2,z:integer;
begin
  d:=35;d2:=100;
//  nilai:=mainsearch(level-2,giliran,_NODEMAX,-_INFINITY-100,_INFINITY+100,data,false);
  z:=80;
  n:=searchroot(level,giliran,_NODEMAX,nilai-d,nilai+d,data,false,0,z);
  n2:=n;
  if (n>=nilai+d) and (n<_INFINITY) then
  begin
     n:=searchroot(level,giliran,_NODEMAX,n2,_INFINITY+500,data,false,0,z);
     if n<n2 then
       n:=searchroot(level,giliran,_NODEMAX,-_INFINITY-500,_INFINITY+500,data,false,0,z);
  end
  else
  if (n<=nilai-d) then
  begin
     inc(retime,rtl div 2);
     n:=searchroot(level,giliran,_NODEMAX,-_INFINITY-500,n,data,false,0,z);
     if n>n2 then
       n:=searchroot(level,giliran,_NODEMAX,-_INFINITY-500,_INFINITY+500,data,false,0,z);
  end;
  result:=n;
end;

function timemanage(giliran:integer;data:tdata):boolean;
var incheck:boolean;
ml:tmovelist;jml,b,a,c:integer;
capPion:boolean;
begin
  incheck:=false;
  bmoves:=_NO_MOVE;
  capPion:=false;
  result:=false;
  if ((giliran=_SISIPUTIH) and (white_checked(data))) or
     ((giliran=_SISIHITAM) and (black_checked(data))) then
     incheck:=true;
  if incheck then
  begin
    if giliran=_SISIPUTIH then white_evasion(data,ml,jml,b)
    else black_evasion(data,ml,jml,b);
    if jml=1 then
    begin
      mmoves:=ml[1].moves;
      result:=true;
      exit;
    end;
  end else
  begin
    if giliran=_SISIPUTIH then
      white_movgen_caps(data,ml,jml)
    else
      black_movgen_caps(data,ml,jml);
    a:=0;
    for b:=1 to jml do
    begin
      if ml[b].score>1 then
      begin
        inc(a);
        c:=a;
      end else
      if ml[b].score>0 then
        capPion:=true;
    end;
    {if a=1 then
      timelimit:=(timelimit * 33 ) div 100
    else
    if a>1 then
      timelimit:=(timelimit * 50 ) div 100
    else
    if capPion then
      timelimit:=(timelimit * 95 ) div 100};
    if timelimit<2 then timelimit:=2;
  end;

end;

function iterative(level,giliran,tipe:integer;data:tdata):integer;
var l,nilai,nilai2,nilai3:integer;
a,b:integer;
dummy:byte;
lasttime:integer;
anu:boolean;
lastmoves,counter:integer;
begin
  l:=5;
  rtl:=timelimit;

  maxdepth:=round(l*2.00);
  nilai3:=0;
  barisitr:=1;
  anu:=false;
  retime:=timelimit;
  ntimelimit:=timelimit div 3;
  maxext:=1.00;


  if timemanage(giliran,data) then exit;

  if not use_mtdf then
  begin
    toplevel:=(l-1) ;
    nilai2:=searchroot((l-1)*FRACTIONAL_PLY,giliran,_NODEMAX,-_INFINITY-500,_INFINITY+500,data,false,0,round(maxext*toplevel*fractional_ply));
    toplevel:=l ;
    nilai:=searchroot(l*FRACTIONAL_PLY,giliran,_NODEMAX,-_INFINITY-500,_INFINITY+500,data,false,0,round(maxext*toplevel*fractional_ply));
  end else
  begin
    toplevel:=l-1;
    nilai2:=mtdf(data,(l-1)*FRACTIONAL_PLY,giliran,0);
    toplevel:=l;
    nilai:=mtdf(data,l*FRACTIONAL_PLY,giliran,0);
  end;
  if testsolved then exit;
  lastmoves:=mmoves;counter:=0;

  while ((usetimer and (timepassed<timelimit)) or (not usetimer and (l<level))) and (l<50) do
  begin
    inc(l);
    toplevel:=l ;
    maxdepth:=round(l*2);
    lasttime:=timepassed;
    newpv:=true;

    if not use_mtdf then
      nilai3:=aspiration(l*FRACTIONAL_PLY,giliran,tipe,nilai2,data)
    else
      nilai3:=mtdf(data,l*FRACTIONAL_PLY,giliran,nilai2);

    if (stop_process) or (timepassed>=timelimit) then
    begin
      result:=nilai;exit;
    end;
{    if (nilai-nilai3>40) and (nilai3<200) and not anu then
    begin
      inc(timelimit,rtl div 2);anu:=true;
    end;
}
    if (nilai3<0) and (nilai>=10) and not anu then
    begin
      inc(timelimit,rtl div 2);
      anu:=true;
    end;

    if (lastmoves=mmoves) and newpv and (l>7) then
    begin
      inc(counter);
      if (counter>=2) and (counter<=12) and (nilai3>-20) then
      begin
         dec(timelimit,round(timelimit*(timereducement[counter]/100)));
      end;
    end else
    begin
      if counter>=4 then
         inc(timelimit,rtl div 8);
      counter:=0;lastmoves:=mmoves;
    end;

    if testsolved then exit;

{    if (usetimer) and (bmoves<>_NO_MOVE) and (timepassed>=ntimelimit) and (nilai3>-10) and (nilai3+30>nilai)
      then
      break;
}
    lasttime:=timepassed-lasttime;
//    if usetimer and ((timelimit-timepassed)*1.2<lasttime) then break;

//    if (nilai3>=_INFINITY-100) then break;
    nilai2:=nilai;
    nilai:=nilai3;
  end;
  toplevel:=level;
  result:=nilai3;
end;


end.
