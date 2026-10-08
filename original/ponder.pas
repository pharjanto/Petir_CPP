unit ponder;


interface
uses classes,header;
type
  tponder=class(tthread)
    constructor create;
    procedure execute;override;
    procedure terminated(sender:tobject);
  end;
  procedure stop_ponder;
  procedure start_ponder;

var
  thr_ponder:tponder;


implementation
uses unit1,hashing_header,uiterative;

procedure stop_ponder;
begin
    stop_process:=true;
    repeat
    until (inpondering=false) or (ponder_type=PONDER_OFF);
end;

procedure start_ponder;
begin
  if ponder_type>PONDER_OFF then
    thr_ponder:=tponder.create;
end;

constructor tponder.create;
begin
  freeonterminate:=true;
  onterminate:=terminated;
  inherited create(false);
end;

procedure initponder;
var a,b:integer;
begin
  stop_process:=false;
  inpondering:=true;
  timelimit:=99999999;
  ai_side:=form1.giliran;
  fillchar(killer0,sizeof(killer0),_NO_MOVE);
  fillchar(killer1,sizeof(killer1),_NO_MOVE);
  for b:=0 to HIST_SIZE do
  begin
//    historyw[b]:=historyw[b] div 12;
  end;

  //cleartable2;
  clearreversetable;
end;

procedure tponder.execute;
var l:integer;
begin
  l:=toplevel;
  initponder;
  iterative(99,form1.giliran,_NODEMAX,data);
//  reversetable;
  toplevel:=l;
  inpondering:=false;
end;

procedure tponder.terminated;
begin
  inpondering:=false;
end;

end.
