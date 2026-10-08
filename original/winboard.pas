unit winboard;

interface
  uses classes;
  type
  twinboard=class(tthread)
    constructor create;
    procedure execute;override;
    procedure   listner_winboard;
    function     readWinboard:string;
  end;
  procedure start_winboard;
  procedure send_winboard(msg:string);
var
 wb:twinboard=nil;
 gHandleout:integer;

implementation

uses sysutils,header,unit3,windows,udata,unit1,tools,hashing_header,repetition,notation,makemove,undoredo,ponder,customboard;
//var giliran:integer;
var forcemode:boolean;

procedure Twinboard.Execute;
begin
  listner_winboard;
end;

function Twinboard.readWinboard:string;   //receive command from winboard
var
  tt:string;
  s:char;
begin
        tt:='';
        repeat
                read(input,s);
                tt:=tt+s;
        until s=#10;
        result:=trim(tt);
end;

procedure parse_timer(st:string);
var i,j,c1,c2,c3:integer;
p1,p2,p3:string;
atime:tdatetime;
Std,Min,
Sec,S1000   : word;
res:integer;
begin
  delete(st,1,6);
  i:=pos(' ',st);
  P1:= Copy(St,1,i-1);

  delete(st,1,i);
  i:=pos(' ',st);
  p2:=copy(st,1,i-1);

  delete(st,1,i);
  p3:=st;
  Val(P1,rmoves,c1);
  Val(P2,j,c2);
  Val(P3,rincrement,c3);
  if c2>0 then
  begin
    atime:=strtotime(p2);
    DecodeTime(ATime,Std,Min,Sec,S1000);
    res:= (Std*60+Min)*timefactor;
  end else
    res:=j*60*timefactor;
  if rmoves=0 then modetime:=MINUTES_GAME
  else modetime:=MOVES_MINUTES;
  rincrement:=rincrement*timefactor;
  if rincrement<>0 then modetime:=MINUTES_GAME_INCREMENT;

  clockwhite:=res;clockblack:=res;
  realclock:=res;
//  form1.memo1.Lines.add(inttostr(res));
  usetimer:=true;
end;

procedure make(move:string;var data:tdata);
var moves:integer;
pp:boolean;
cp:shortint;
j:byte;
begin
  stop_ponder;
  moves:=notationtomove(move,data,form1.giliran);
  j:=0;

  if form1.giliran=_sisiputih then
    makewhitemove(moves,data)
  else
    makeblackmove(moves,data);
  if addrephash(data.hashkey,2-form1.giliran)=2 then
  begin
    send_winboard('1/2-1/2');
  end;
  form1.giliran:=3-form1.giliran;
  addundo(undo,data,clockwhite,clockblack);
  inc(jumlahlangkah);
   if (modetime=MOVES_MINUTES) and (jumlahlangkah>=rmoves*2) then
   begin
     jumlahlangkah:=0;
   end;

end;

function gettimer(tm:string):integer;
var t,x:integer;
begin
  delete(tm,1,5);
  val(tm,t,x);
  result:=(t div 100) * timefactor; 
end;

procedure Twinboard.listner_winboard;
var tt:string;
computer_side:byte;
wtm:boolean;
begin
  while 1=1 do
  begin
    tt:=readwinboard;
//    form3.Memo1.Lines.add(tt);
    if tt='quit' then
    begin
        usewinboard:=false;
        exit;
    end;
    if copy(tt,1,8)='protover' then
    begin
        send_winboard('feature myname="Petir 1.0" analyze=0 time=1 name=1 san=0 setboard=1');
    end;
    if copy(tt,1,3)='new' then
    begin
      stop_process:=true;
      repeat
      until not computer_thinking;
      stop_ponder;
      form1.start_new_game;
      if usetimer then
      begin
        clockblack:=realclock;
        clockwhite:=realclock;
      end;
    end;
    if copy(tt,1,8)='setboard' then
    begin
      readnotation(copy(tt,10,length(tt)-9),data,wtm);
      if wtm then form1.giliran:=_SISIPUTIH else form1.giliran:=_SISIHITAM;
      fillbitboard(data);
      form1.gambar(data,200);
    end;
    if copy(tt,1,4)='hard' then
    begin
      ponder_type:=PONDER_OFF;
    end;
    if copy(tt,1,4)='easy' then
    begin
      stop_ponder;
      ponder_type:=PONDER_OFF;
    end;
    if copy(tt,1,4)='time' then
    begin
      if computer_side=_sisiputih then
      begin
        clockwhite:=gettimer(tt);
      end
      else
      begin
        clockblack:=gettimer(tt);
      end;
    end;{
    if copy(tt,1,4)='otim' then
    begin
      if computer_side=_sisiputih then clockblack:=gettimer(tt)
      else clockwhite:=gettimer(tt);
    end; }

    if copy(tt,1,5)='force' then
      forcemode:=true;
    if copy(tt,1,2)='st' then
    begin
      usetimer:=true;

      modetime:=SECONDS_MOVE;
      realclock:=strtoint(copy(tt,4,length(tt)-3))*timefactor;
      clockwhite:=realclock;clockblack:=realclock;
      form1.Caption:=inttostr(realclock);
    end else
    if copy(tt,1,5)='level' then
    begin
      parse_timer(tt); //pake timer
    end else
    if copy(tt,1,2)='go' then
    begin
       forcemode:=false;
       computer_side:=form1.giliran;
       thr_search:=tsearch.create;
    end;
    if (tt[2] in ['1'..'8']) and (tt[1] in ['a'..'h']) then
    begin
      make(tt,data);

//\      form1.gambar(data,200);
//      computer_side:=giliran;
      if not forcemode then thr_search:=tsearch.create;
    end;

  end;
end;

procedure start_winboard;
begin
  forcemode:=false;
  wb:=twinboard.create;
  wb.resume;
//  forcemode:=false;
  gHandleout:=getstdhandle(std_output_handle);
  usewinboard:=true;
end;

procedure send(var msg:string);
var len:cardinal;
    x:pChar;
begin
   len:=length(msg);
   x:=pchar(msg+#10);
   _lwrite(gHandleout,x,len+1);
end;

procedure send_winboard(msg:string); 
begin
    send(msg);
end;


constructor Twinboard.Create;
begin
    inherited create(true);
end;


end.
