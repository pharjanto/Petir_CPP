unit perft_test;

interface
uses header,classes;

type
tperft=class(tthread)

    constructor create;
    procedure execute;override;
    procedure terminated(sender:tobject);
end;

procedure perft(level:integer;data:tdata;giliran:integer);
procedure killperftthread;

var perft_result:int64;
    thr_perft:tperft;
    perft_depth:integer;

implementation
uses makemove,movgen,tools,windows,udata,sysutils,unit1;


constructor tperft.create;
begin
  freeonterminate:=true;
  onterminate:=terminated;
  inherited create(false);
end;

procedure killperftthread;
begin
  if thr_perft<>nil then
  begin
    stop_process:=true;
    thread_killed:=false;
    repeat
    until thread_killed;
    stop_process:=false;

  end;
end;

procedure tperft.execute;
var n:integer;ref:longint;
begin
 stop_process:=false;
 for n:=1 to perft_depth do
 begin
   perft_result:=0;
   ref:=gettickcount;
   perft(n,data,_SISIPUTIH);
   if stop_process then
   begin
     thread_killed:=true;
     break;
   end;
   ref:=gettickcount-ref;
 end;

end;

procedure tperft.terminated;
begin

 thread_killed:=true;

end;

procedure perft;
var jml,p,sc:integer;
temp:tdata;
ml:tmovelist;
t:boolean;
begin
  if level=0 then
  begin
    inc(perft_result);
  end else
  begin
    temp:=data;
    if stop_process then
    begin
      inc(perft_result);
      exit;
    end;
    if giliran=_SISIPUTIH then
    begin
      if not white_checked(temp) then
      begin
        white_movgen_caps(data,ml,jml);
        for p:=1 to jml do
        begin
//               temp2:=p;
                makewhitemove(ml[p].moves,data);
                if white_checked(data) then begin
                  data:=temp;continue;
                end;
                perft(level-1,data,3-giliran);
                data:=temp;
        end;
        white_movgen_noncaps(data,ml,jml,0);
        for p:=1 to jml do
        begin
                makewhitemove(ml[p].moves,data);
                if white_checked(data) then begin
                  data:=temp;continue;
                end;

//                temp2:=p;
                perft(level-1,data,3-giliran);
                data:=temp;
        end;
      end else
      begin
        white_evasion(data,ml,jml,sc);
        for p:=1 to jml do
        begin
                makewhitemove(ml[p].moves,data);
                perft(level-1,data,3-giliran);
                data:=temp;
        end;
      end;
    end else
    begin
      if not black_checked(temp) then
      begin
        black_movgen_caps(data,ml,jml);
        for p:=1 to jml do
        begin
                makeblackmove(ml[p].moves,data);
                if black_checked(data) then
                begin
                  data:=temp;continue;
                end;
                perft(level-1,data,3-giliran);
                data:=temp;
        end;
        black_movgen_noncaps(data,ml,jml);
        for p:=1 to jml do
        begin
                makeblackmove(ml[p].moves,data);
                if black_checked(data) then
                begin
                  data:=temp;continue;
                end;

                perft(level-1,data,3-giliran);
                data:=temp;
        end;
      end else
      begin
        black_evasion(data,ml,jml,sc);
        for p:=1 to jml do
        begin
                makeblackmove(ml[p].moves,data);
{                if black_checked(data) then
                begin
                  data:=temp;continue;
                end;
}                perft(level-1,data,3-giliran);
                data:=temp;
        end;

      end;
    end;

  end;
end;

end.
