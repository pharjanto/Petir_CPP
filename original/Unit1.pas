unit Unit1;
// Petir 1.0
{
important note :
  - lastbitp kebalik
  - pengecekan backward pawn harus ditambah tidak boleh untuk pion bebas
  - evaluation extension gak boleh dilakukan kalau lagi incheck
  - pion bebas scale harus dikurangi
  - raja di dekat pion bebas harus dikurangi
  - tempi = 0 pada serangan raja
  -  if (data.nilai_perwira_putih=6) and (data.Nilai_perwira_hitam=6) then
  - and (nodetype<>NODE_PV) di null move
  - menskak2 diganti menskak

          if (pionbebashitam and bit8mask[tempx-1]<>0) then
          begin
            if  (data.papan[pos-1]=_PIONHITAM) then
              inc(temp,NILAI_PION_BEBAS_CONNECTED3+20)  //+ 20 harus di sini
            else
            if (data.papan[pos-9]=_PIONHITAM)  then
              inc(temp,NILAI_PION_BEBAS_CONNECTED3);
          end;
        end;
        if tempx<7 then
        begin
          if (pionbebashitam and bit8mask[tempx+1]<>0)then
          begin
            if (data.papan[pos+1]=_PIONHITAM) then
              inc(temp,NILAI_PION_BEBAS_CONNECTED3+20) else
            if (data.papan[pos-7]=_PIONHITAM) then
              inc(temp,NILAI_PION_BEBAS_CONNECTED3);

          end;

  if (data.bc=NOROKADE) and (((data.papan[g7]=0) and (data.papan[g6]=0))
  and ((data.papan[b7]=0) and (data.papan[b6]=0))) and (data.queenwhite<>0)
  and (data.flagrokade and bithitamnoc<>0)
  then
    inc(nilai,20);

  if (data.wc=NOROKADE) and (((data.papan[g2]=0) and (data.papan[g3]=0))
  and ((data.papan[b2]=0) and (data.papan[b3]=0))) and (data.queenblack<>0)
  and (data.flagrokade and bitputihnoc<>0)
  then
    dec(nilai,20);          
  

}
interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls,udata, StdCtrls, ToolWin, ComCtrls, Menus,header, Spin,hashing,
  ImgList,undoredo;

const _SIZE=49;_POSATAS:integer=24;_POSKIRI:integer=24;
      _NOTMOVEPIECE=200;


type
  tsearch=class(tthread)
    constructor create;
    procedure execute;override;
    procedure terminated(sender:tobject);
  end;

  tsetting=record
    version:integer;
    maxhash:integer;
    futpruningOK,lazyOK,qpruningOK,nullOK:boolean;
    lazy_margin,futil_margin,q_margin:integer;
    thread_priority:integer;
    FRACTIONAL_PLY:integer;
    SKAK_EXTENSION:integer;
    MATE_THREAT_EXTENSION:integer;
    SINGLE_REPLY_EXTENSION:integer;
    PUSHED_PAWN_EXTENSION_MIDGAME:integer;
    PUSHED_PAWN_EXTENSION_ENDGAME:integer;
    PUSHED_PAWN_EXTENSION_LATE_ENDGAME:integer;

  end;

  tsav=record
    data:tdata;giliran:integer;
    players:integer;
    usetimer:boolean;clockputih,clockhitam:integer;
    undo:tundo;
  end;


  TForm1 = class(TForm)
    Image2: TImage;
    Imageself: TImage;
    Panel3: TPanel;
    Image1: TImage;
    StatusBar1: TStatusBar;
    ToolBar1: TToolBar;
    MainMenu1: TMainMenu;
    Game1: TMenuItem;
    New1: TMenuItem;
    N1: TMenuItem;
    Load1: TMenuItem;
    Saveas1: TMenuItem;
    Save1: TMenuItem;
    About1: TMenuItem;
    N2: TMenuItem;
    Exit1: TMenuItem;
    Action1: TMenuItem;
    Undo1: TMenuItem;
    Redo1: TMenuItem;
    N3: TMenuItem;
    esting1: TMenuItem;
    BenchMark1: TMenuItem;
    Perft1: TMenuItem;
    OpenDialog1: TOpenDialog;
    SaveDialog1: TSaveDialog;
    TimerMSG: TTimer;
    Level1: TMenuItem;
    imer1: TMenuItem;
    Fixeddepth1: TMenuItem;
    N15mingame1: TMenuItem;
    N5mingame1: TMenuItem;
    N60mingame1: TMenuItem;
    custom1: TMenuItem;
    N41: TMenuItem;
    N51: TMenuItem;
    N61: TMenuItem;
    N71: TMenuItem;
    N81: TMenuItem;
    N91: TMenuItem;
    N101: TMenuItem;
    Custom2: TMenuItem;
    EditPosition1: TMenuItem;
    N4: TMenuItem;
    Evaluate1: TMenuItem;
    Switchside1: TMenuItem;
    Window1: TMenuItem;
    SearchStatistic1: TMenuItem;
    ImageList1: TImageList;
    ToolButton1: TToolButton;
    ToolButton2: TToolButton;
    ToolButton3: TToolButton;
    ToolButton4: TToolButton;
    ToolButton5: TToolButton;
    ToolButton6: TToolButton;
    ToolButton7: TToolButton;
    ToolButton8: TToolButton;
    ChangePlayer1: TMenuItem;
    ToolButton9: TToolButton;
    Information1: TMenuItem;
    Timer1: TTimer;
    Iterative1: TMenuItem;
    Clock1: TMenuItem;
    EvasionTest1: TMenuItem;
    InputMove1: TMenuItem;
    N5: TMenuItem;
    Book1: TMenuItem;
    LearnPGN1: TMenuItem;
    OpenDialog2: TOpenDialog;
    ForceMove1: TMenuItem;
    Editor1: TMenuItem;
    FEN1: TMenuItem;
    procedure Image1MouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure FormCreate(Sender: TObject);
    procedure FormActivate(Sender: TObject);
    procedure Image1MouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure Image1MouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure New1Click(Sender: TObject);
    procedure Perft1Click(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure Saveas1Click(Sender: TObject);
    procedure Load1Click(Sender: TObject);
    procedure tulisdiskak(giliran:integer);
    procedure TimerMSGTimer(Sender: TObject);
    procedure N41Click(Sender: TObject);
    procedure Custom2Click(Sender: TObject);
    procedure N5mingame1Click(Sender: TObject);
    procedure custom1Click(Sender: TObject);
    procedure EditPosition1Click(Sender: TObject);
    procedure Evaluate1Click(Sender: TObject);
    procedure BenchMark1Click(Sender: TObject);
    procedure Switchside1Click(Sender: TObject);
    procedure SearchStatistic1Click(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure About1Click(Sender: TObject);
    procedure ChangePlayer1Click(Sender: TObject);
    procedure ToolButton9Click(Sender: TObject);
    procedure Information1Click(Sender: TObject);
    procedure Timer1Timer(Sender: TObject);
    procedure Clock1Click(Sender: TObject);
    procedure Iterative1Click(Sender: TObject);
    procedure Undo1Click(Sender: TObject);
    procedure Redo1Click(Sender: TObject);
    procedure Save1Click(Sender: TObject);
    procedure InputMove1Click(Sender: TObject);
    procedure LearnPGN1Click(Sender: TObject);
    procedure EvasionTest1Click(Sender: TObject);
    procedure FormPaint(Sender: TObject);
    procedure Evasion1Click(Sender: TObject);
    procedure ForceMove1Click(Sender: TObject);
    procedure Editor1Click(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure FEN1Click(Sender: TObject);

  private
    lastsavedfile:string;
    procedure make(moves:integer);
    { Private declarations }
  public
    defaultdir:string;
    chess_sets,giliran:byte;

    procedure init_image;
    procedure gambar(var data:tdata;nodraw:byte);
    procedure gambar1kotak(data:tdata;a,b:byte);
    Procedure gambar_piece_moving(x,y:integer;piece:shortint);
    procedure init_board;
    procedure init_var;
    procedure start_new_game;
    procedure init_search;
    function is_move_legal(var moves:integer;var giliran:byte):boolean;
    procedure freeimage;
    procedure showstat;
    procedure tulisgiliran;
    procedure tulislevel;

    { Public declarations }
  end;

  function fstring(s:string;w:byte):string;

var
  thr_search:tsearch;
  Form1: TForm1;

  level,nilai:integer;
  intvmsg:integer;
  g_latar_pion,g_pion_putih,g_pion_hitam:tbitmap;
  g_latar_benteng,g_benteng_putih,g_benteng_hitam:tbitmap;
  g_latar_kuda,g_kuda_putih,g_kuda_hitam:tbitmap;
  g_latar_gajah,g_gajah_putih,g_gajah_hitam:tbitmap;
  g_latar_mentri,g_mentri_putih,g_mentri_hitam:tbitmap;
  g_latar_raja,g_raja_putih,g_raja_hitam:tbitmap;
//  white_time,black_time:integer;
  undo:tundo;

implementation
uses movgen, Unitnewgame,bitboard,makemove,perft_test,search,evalmask,tools,
  unitsetupboard,ueval, Unit2, Unit3, Unit4,valid, Unit5, Unit6,uiterative,
  Unit7,hashing_header, Unit8,notation,usee,unit9,repetition, Unit10,opening_book,
  Unit11,test_suites,winboard,ponder, Unit12,clipbrd,root;


var
  moving:boolean;
  piece_moving:shortint;
  x_asal,y_asal:byte;
  piece_rect:trect;


{$R *.dfm}

procedure tform1.tulislevel;
begin
  statusbar1.panels[2].text:=inttostr(toplevel)+' ply'
end;

procedure stop_thinking;
begin
    stop_process:=true;
    repeat
    until computer_thinking=false;
end;

function fstring(s:string;w:byte):string;
var a,n:integer;
t:string;
begin
  t:=s;
  n:=w-length(s);
  for a:=1 to n do
    t:=t+' ';
  fstring:=t;
end;

procedure updatetimer(giliran:byte);
begin
  if usetimer then
  begin
     if modetime=MINUTES_GAME_INCREMENT then
     begin
       if giliran=_SISIPUTIH then inc(clockwhite,rincrement)
       else inc(clockblack,rincrement);
     end;
     if (modetime=SECONDS_MOVE) then
     begin
        clockblack:=realclock;
        clockwhite:=realclock;
     end;
     if (modetime=MOVES_MINUTES) and (jumlahlangkah>=rmoves*2) then
     begin
       clockwhite:=realclock;
       clockblack:=realclock;
       jumlahlangkah:=0;
     end;
  end;

end;


procedure killallthread;
begin
end;

procedure tform1.tulisgiliran;
begin
  if giliran=_SISIPUTIH then
    StatusBar1.Panels[1].Text:='White to move' else
  statusbar1.panels[1].text:='Black to move';
end;

procedure tsearch.terminated;
begin
  if not stop_process then
  begin
    form1.giliran:=3-form1.giliran;
    form1.gambar(data,_NOTMOVEPIECE);
    form1.tulisgiliran;
    form1.StatusBar1.Panels[3].Text:='';
    computer_thinking:=false;
  end;
  start_ponder;
end;

function settimelimit(giliran:integer):integer;
begin
  if (modetime=MINUTES_GAME) or (modetime=MINUTES_GAME_INCREMENT) then
  begin
    if (giliran=_SISIHITAM) then
    begin
      if jumlahlangkah<=80 then
        result:=clockblack div (60-jumlahlangkah div 2)-2
      else
      begin
        result:=clockblack div 30;
      end;
    end else
    begin
      if jumlahlangkah<=80 then
        result:=clockwhite div (60-jumlahlangkah div 2)-2
      else
      begin
        result:=clockwhite div 30;
      end;
    end;
    if result=0 then result:=1;
  end else
  if (modetime=MOVES_MINUTES) then
  begin
    if giliran=_SISIHITAM then
      result:=clockblack div ((rmoves+2)-(jumlahlangkah div 2))
    else
      result:=clockwhite div ((rmoves+2)-(jumlahlangkah div 2));
  end else
  if modetime=SECONDS_MOVE then
    result:=realclock;
//  form1.caption:=inttostr(result div 4);
end;


procedure tform1.init_search;
var a,b:integer;
begin
//  fl:=0;fh:=0;pvn:=0;
  evalms:=0;
  perft_result:=0;
  totalAddHash:=0;
  Collision:=0;
  stalemate:=false;
  lazyts:=0;
  maxdepth1:=0;evalsee:=0;movgenms:=0;

  cetc:=0;
  futfail:=0;
  futTry:=0;
  total_node:=0;
  maxdepth:=0;
  ai_side:=giliran;
  hash_hit:=0;
  timepassed:=0;
  lazy_margin1a:=lazy_margin1;
  lazy_margin2a:=lazy_margin2;
  lazy_margin3a:=lazy_margin3;
  testsolved:=false;
  clearkiller;
  nerror:=0;


  if ponder_type=PONDER_OFF then
    cleartable2
  else
    clearreversetable;
  co_hash:=0;totalco:=0; hashco:=0;
  lazyexit:=0;
  fpruning:=0;efpruning:=0;razor:=0;
  cofirst:=0;
  computer_thinking:=true;

  for a:=-6 to 6 do
  for b:=0 to 67 do
  begin
    historyw[a,b]:=historyw[a,b] div 256;
    historyb[a,b]:=historyb[a,b] div 256;
  end;
  for a:=1 to formiterative.StringGrid1.RowCount-1 do
  for b:=0 to formiterative.StringGrid1.colcount-1 do
    formiterative.StringGrid1.Cells[b,a]:='';
  startclock:=gettickcount;
  timelimit:=999999999;
  qnode:=0;

  if usetimer then
  begin
    timelimit:=settimelimit(giliran);
  end;

end;


procedure tform1.showstat;
var n,a,b,kosong:integer;
begin
 n:=16;
 with form3 do
 begin
   try
    memo1.lines.clear;
    Memo1.Lines.add(fstring('Total node',n)+ ': '+inttostr(total_node));
    if ref2>0then Memo1.Lines.add(fstring('NPS',n)+': '+inttostr(round(total_node/ref2*1000)));
    Memo1.Lines.add(fstring('Waktu ',n)+': '+inttostr(ref2)+' ms');
    Memo1.Lines.add(fstring('Lazy',n)+': '+inttostr(lazyexit));
    Memo1.Lines.add(fstring('Fut Pruning ',n)+': '+inttostr(fpruning));

    if totalco>0 then Memo1.Lines.add(fstring('Cutoff first ',n)+': '+floattostr((cofirst/totalco*100))+' %');
    Memo1.Lines.add(fstring('research ',n)+': '+inttostr(rs));
    Memo1.Lines.add(fstring('prune try ',n)+': '+inttostr(futtry));
    Memo1.Lines.add(fstring('prune fail ',n)+': '+inttostr(futfail));
    Memo1.Lines.add(fstring('max depth ',n)+': '+inttostr(maxdepth1));
    if futtry>0then Memo1.Lines.add(fstring('q/total',n)+': '+floattostr((futfail/futtry*100)));
    kosong:=0;

    Memo1.Lines.add(fstring('hash util ',n)+': '+floattostr((kosong/maxhash*100))+' %');
    Memo1.Lines.add(fstring('total add ',n)+': '+inttostr(totalAddHash)+' '+inttostr(collision));
    Memo1.Lines.add(fstring('eval ms ',n)+': '+inttostr(evalms)+' '+inttostr(evalsee)+' '+inttostr(movgenms));

    memo1.lines.add('');
    {
    for a:=0 to 67 do
    for b:=0 to 67 do
      if rootorder[a,b]<>0 then
        memo1.lines.add(inttostr(rootorder[a,b])+' '+getnotasi(a)+' '+getnotasi(b)+' ');
    }
//    Memo1.Lines.add(fstring('max depth ',n)+': '+inttostr(depthmax));

   // Memo1.Lines.add(fstring('Cutoff Node',n)+': '+inttostr(round(totalco/total_node*100))+'%');
   finally
   end;
 end;

end;

function checkresult(data:tdata):boolean;
var ml:tmovelist;jml,sc:integer;
a:boolean;
begin
  result:=false;
  if isdraw(data,a) then
  begin
    form1.statusbar1.Panels[0].Text:='Draw by insufficient material';
    form1.giliran:=_GAMEOVER;
    form1.gambar(data,200);
    if usewinboard then send_winboard('1/2-1/2 {draw by insufficient material}');
    result:=true;
  end;
  if stalemate then
  begin
    form1.statusbar1.Panels[0].Text:='Draw by stalemate';
    form1.giliran:=_GAMEOVER;
    form1.gambar(data,200);
    if usewinboard then send_winboard('1/2-1/2');
    result:=true;
  end;

  if white_checked(data) then
  begin
    white_evasion(data,ml,jml,sc);
    if jml=0 then
    begin
      form1.statusbar1.Panels[0].Text:='White checkmated';
      form1.giliran:=_GAMEOVER;
      if usewinboard then send_winboard('0-1 {white checkmated}');
      form1.gambar(data,200);
      result:=true;
    end;
  end;
  if black_checked(data) then
  begin
    black_evasion(data,ml,jml,sc);
    if jml=0 then
    begin
      form1.statusbar1.Panels[0].Text:='Black checkmated ';
      if usewinboard then send_winboard('result 1-0 {black checkmated}');
      form1.giliran:=_GAMEOVER;
      form1.gambar(data,200);
      result:=true;
    end;
  end;
end;

Procedure tsearch.execute;
var pp:boolean;
a,ep:byte;
s:string;
m:integer;
begin
  tropismeval:=0;
  if form1.giliran=_GAMEOVER then exit;
  stop_process:=false;
//  toplevel:=6;
  level:=toplevel;
  setthreadpriority(thr_search.Handle,thread_priority);
  form1.StatusBar1.Panels[3].Text:='Computer is thinking';

  form1.init_search;
  ref2:=gettickcount;
  mmoves:=_NO_MOVE;

// nilai:=mainsearch(level*fractional_ply,form1.giliran,_NODEMAX,-_INFINITY-100,_INFINITY+100,data,false,0,a);
  if outofbook<=3 then
    mmoves:=searchbook(data,form1.giliran);
  if mmoves=_NO_MOVE then
  begin
    inc(outofbook);
    nilai:=iterative(level,form1.giliran,_NODEMAX,data);

    if (nilai>=-15) and (nilai<=5) and (data.nilai_perwira_putih<=9) and (data.nilai_perwira_hitam<=9)
    then inc(draw_count);
    if draw_count>=4 then
      if usewinboard then send_winboard('offer draw');     

    if nilai<=resign_value then inc(resign_count);
    if resign_count>=3 then
    begin
       if usewinboard then
       begin
         if ai_side=_SISIPUTIH then
           send_winboard('0-1 {white resign}')
         else
           send_winboard('1-0 {black resign}')
       end
       else
         form1.StatusBar1.Panels[0].Text:='Computer Resign';
    end;
  end;

  if stop_process then
  begin
    computer_thinking:=false;
    exit;
  end;
  ref2:=gettickcount-ref2;
  ep:=data.ep;

  if form1.giliran=_SISIHITAM then
  begin
    makeblackmove(mmoves,data);
  end else
  begin
    makewhitemove(mmoves,data);
  end;

  if (data.nilai_perwira_putih=0) and (data.nilai_perwira_hitam=0) and
  (data.cp>=_KUDAPUTIH) and (data.cp<=_MENTRIPUTIH)
  then
  begin
    data.pawnending:=true;
  end;


  if usewinboard then
  begin
    s:=movetonotation(mmoves,ep,form1.giliran);
//    form1.Caption:=s;
    send_winboard('move '+s);
  end;
  if data.move50count>=101 then
  begin
    if usewinboard then
      send_winboard('1/2-1/2 {draw by 50 move rule}')
    else
      form1.statusbar1.Panels[0].Text:='Draw by 50 move rule';
  end;

  addundo(undo,data,clockwhite,clockblack);
  if checkresult(data) then exit;
  if addrephash(data.hashkey,2-form1.giliran)=2 then
  begin
    if not usewinboard then
    begin
      form1.statusbar1.Panels[0].Text:='Draw by repetition';
      form1.giliran:=_GAMEOVER;
      form1.gambar(data,200);
    end else send_winboard('1/2-1/2');
    exit;
  end;
  inc(jumlahlangkah);
  if not usewinboard then
    form1.showstat;
  updatetimer(form1.giliran);
end;

procedure tform1.tulisdiskak(giliran:integer);
begin
  if giliran=_SISIPUTIH then
    statusbar1.Panels[0].Text:='Illegal move, white is checked'
  else statusbar1.Panels[0].Text:='Illegal move, Black is checked';
  intvmsg:=0;
  timermsg.Enabled:=true;
end;

function tform1.is_move_legal;
var sfrom,sto:integer;
begin
  sfrom:=moves and 127;
  sto:=(moves shr 7) and 127;
  if (sto>63) or (sto<0) or
  (sto div 8 <0) or (sto div 8 >7) and not (sto mod 8 in [0..7]) then
  begin
    result:=false;exit;
  end;
  if (data.papan[sfrom]=_RAJAPUTIH) and (sfrom=e1) and (sto=g1) then
    sto:=_ROKADEPENDEK;
  if (data.papan[sfrom]=_RAJAPUTIH) and (sfrom=e1) and (sto=c1) then
    sto:=_ROKADEPANJANG;
  if (data.papan[sfrom]=_RAJAHITAM) and (sfrom=e8) and (sto=g8) then
    sto:=_ROKADEPENDEK;
  if (data.papan[sfrom]=_RAJAHITAM) and (sfrom=e8) and (sto=c8) then
    sto:=_ROKADEPANJANG;
  if (data.papan[sfrom]=_PIONPUTIH) and ((sto-sfrom=9) or (sto-sfrom=7))
  and (data.papan[sto]=0) and (sfrom div 8 =4) then sto:=_EN_PASSANT;
  if (data.papan[sfrom]=_PIONHITAM) and ((sfrom-sto=9) or (sfrom-sto=7))
  and (data.papan[sto]=0) and (sfrom div 8 =3) then sto:=_EN_PASSANT;

  if ((data.papan[sfrom]=_PIONPUTIH) and (sfrom div 8 = 6) and (data.papan[sto]<=0)) or
     ((data.papan[sfrom]=_PIONHITAM) and (sfrom div 8 = 1) and (data.papan[sto]>=0))
  then
  begin
    formpromotion.showmodal;
    moves:=sfrom+sto shl 7+formpromotion.promosi shl 14;
  end else
    moves:=sfrom+sto shl 7;
  result:=isvalid(data,sfrom+sto shl 7,giliran);

end;

procedure delete_data;
begin
 fillchar(historyw,sizeof(historyw),0);
 fillchar(historyb,sizeof(historyb),0);
 clearkiller;
 cleartable;
 initrepdata;
end;

procedure tform1.Start_New_Game;
begin
 initstate(data);
 outofbook:=0;
 resign_count:=0;
 draw_count:=0;
 giliran:=_SISIPUTIH;
 form1.gambar(data,200);
 tulisgiliran;
 delete_data;
 jumlahlangkah:=0;
 initrepdata;
 resetundo(undo);
end;


procedure tform1.init_var;
begin
 timefactor:=4;
 runningtest:=false;
 form1.defaultdir:=getcurrentdir;
 chess_sets:=1;
 usetimer:=false;
 moving:=false;
 init_mask;
 killer0[-2]:=0;
 killer1[-2]:=0;
 killer0[-1]:=0;
 killer1[-1]:=0;
 inithashkey;
 inittable;
 initevalmask;
 players:=_PLAYERVSCOMP;
 toplevel:=10;
 tulislevel;
 form1.lastsavedfile:='';
 clockwhite:=0;clockblack:=0;
 thread_priority:=THREAD_PRIORITY_NORMAL;
 nullOK:=true;lazyOK:=true;futpruningOK:=true;qpruningOK:=true;
 resetundo(undo);
 if use_book then
 begin
   initbook;
   randomize;
   loadbook;
 end;
 
end;

procedure tform1.init_image;
begin
    g_latar_raja:=tbitmap.create;
   g_latar_raja.loadfromfile(defaultdir+'\chessset\'+inttostr(chess_sets)+'\s_raja.bmp');
   g_raja_putih:=tbitmap.create;
   g_raja_putih.loadfromfile(defaultdir+'\chessset\'+inttostr(chess_sets)+'\raja.bmp');
   g_raja_hitam:=tbitmap.create;
   g_raja_hitam.loadfromfile(defaultdir+'\chessset\'+inttostr(chess_sets)+'\raja_h.bmp');

   g_latar_mentri:=tbitmap.create;
   g_latar_mentri.loadfromfile(defaultdir+'\chessset\'+inttostr(chess_sets)+'\s_mentri.bmp');
   g_mentri_putih:=tbitmap.create;
   g_mentri_putih.loadfromfile(defaultdir+'\chessset\'+inttostr(chess_sets)+'\mentri.bmp');
   g_mentri_hitam:=tbitmap.create;
   g_mentri_hitam.loadfromfile(defaultdir+'\chessset\'+inttostr(chess_sets)+'\mentri_h.bmp');

   g_latar_gajah:=tbitmap.create;
   g_latar_gajah.loadfromfile(defaultdir+'\chessset\'+inttostr(chess_sets)+'\s_gajah.bmp');
   g_gajah_putih:=tbitmap.create;
   g_gajah_putih.loadfromfile(defaultdir+'\chessset\'+inttostr(chess_sets)+'\gajah.bmp');
   g_gajah_hitam:=tbitmap.create;
   g_gajah_hitam.loadfromfile(defaultdir+'\chessset\'+inttostr(chess_sets)+'\gajah_h.bmp');

   g_latar_kuda:=tbitmap.create;
   g_latar_kuda.loadfromfile(defaultdir+'\chessset\'+inttostr(chess_sets)+'\s_kuda.bmp');
   g_kuda_putih:=tbitmap.create;
   g_kuda_putih.loadfromfile(defaultdir+'\chessset\'+inttostr(chess_sets)+'\kuda.bmp');
   g_kuda_hitam:=tbitmap.create;
   g_kuda_hitam.loadfromfile(defaultdir+'\chessset\'+inttostr(chess_sets)+'\kuda_h.bmp');

   g_latar_benteng:=tbitmap.create;
   g_latar_benteng.loadfromfile(defaultdir+'\chessset\'+inttostr(chess_sets)+'\s_benteng.bmp');
   g_benteng_putih:=tbitmap.create;
   g_benteng_putih.loadfromfile(defaultdir+'\chessset\'+inttostr(chess_sets)+'\benteng.bmp');
   g_benteng_hitam:=tbitmap.create;
   g_benteng_hitam.loadfromfile(defaultdir+'\chessset\'+inttostr(chess_sets)+'\benteng_h.bmp');
   g_pion_hitam:=tbitmap.create;
   g_pion_hitam.loadfromfile(defaultdir+'\chessset\'+inttostr(chess_sets)+'\pion_h.bmp');
   g_pion_putih:=tbitmap.create;
   g_pion_putih.loadfromfile(defaultdir+'\chessset\'+inttostr(chess_sets)+'\pion.bmp');
   g_latar_pion:=tbitmap.create;
   g_latar_pion.loadfromfile(defaultdir+'\chessset\'+inttostr(chess_sets)+'\s_pion.bmp');

end;

procedure TForm1.Image1MouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
var x1,y1:byte;
dest:trect;
begin
if (button=mbleft) and not usewinboard then begin
 x1:=(x-_POSKIRI) div _SIZE;
 y1:=(y-_POSATAS) div _SIZE;
// moving:=true;
 if not usewinboard and
 not computer_thinking and not moving and (data.papan[map(7-y1,x1)]<>0) and (x1 in [0..7])
 and (y1 in [0..7])
 and
 (((giliran=_SISIPUTIH) and (data.papan[map(7-y1,x1)]>0) )
 or ((giliran=_SISIHITAM) and (data.papan[map(7-y1,x1)]<0)))
 then begin

   moving:=true;
   piece_moving:=data.papan[map(7-y1,x1)];//mencatat biji apa yang dipindahkan

   x_asal:=x1;
   y_asal:=7-y1;

   gambar(data,map(7-y1,x1));

   dest:=rect(0,0,60,60);
   piece_rect:=rect(x-30,y-30,x+30,y+30);//menentukan kotak berukuran 40*40 di sekitar posisi mouse

   form1.image1.canvas.copymode:=cmsrccopy;
   image2.canvas.copyrect(dest,form1.image1.canvas,piece_rect);//mengcopy kotak 40*40 di sekitar posisi mouse pada form.canvas ke image.canvas

   gambar_piece_moving(x-20,y-20,piece_moving);
 end;
end;
end;

procedure tform1.freeimage;
begin
 with form1 do begin
  g_latar_raja.free;
  g_raja_putih.free;
  g_raja_hitam.free;
  g_latar_mentri.free;
  g_mentri_putih.free;
  g_mentri_hitam.free;
  g_latar_gajah.free;
  g_gajah_putih.free;
  g_gajah_hitam.free;
  g_Latar_kuda.free;
  g_kuda_putih.free;
  g_kuda_hitam.free;
  g_benteng_putih.free;
  g_benteng_hitam.free;
  g_latar_benteng.free;
  g_latar_pion.free;
  g_pion_putih.free;
  g_pion_hitam.free;
 end;
end;

Procedure tform1.gambar_piece_moving;
begin
  image1.Canvas.Lock;
  with form1.image1.canvas do begin
      if piece=_PIONPUTIH then begin
         copymode:=cmsrcand;         draw(x,y,g_latar_pion);
         copymode:=cmsrcpaint;         draw(x,y,g_pion_putih);
      end;
      if piece=_PIONHITAM then begin
         copymode:=cmsrcand;         draw(x,y,g_latar_pion);
         copymode:=cmsrcpaint;         draw(x,y,g_pion_hitam);
      end;
      if piece=_BENTENGPUTIH then begin
         copymode:=cmsrcand;         draw(x,y,g_latar_benteng);
         copymode:=cmsrcpaint;         draw(x,y,g_benteng_putih);
      end;
      if piece=_BENTENGHITAM then begin
         copymode:=cmsrcand;         draw(x,y,g_latar_benteng);
         copymode:=cmsrcpaint;         draw(x,y,g_benteng_hitam);
      end;
      if piece=_KUDAHITAM then begin
         copymode:=cmsrcand;         draw(x,y,g_latar_kuda);
         copymode:=cmsrcpaint;         draw(x,y,g_kuda_hitam);
      end;
      if piece=_KUDAPUTIH then begin
         copymode:=cmsrcand;         draw(x,y,g_latar_kuda);
         copymode:=cmsrcpaint;         draw(x,y,g_kuda_putih);
      end;
      if piece=_GAJAHHITAM then begin
         copymode:=cmsrcand;         draw(x,y,g_latar_gajah);
         copymode:=cmsrcpaint;         draw(x,y,g_gajah_hitam);
      end;
      if piece=_GAJAHPUTIH then begin
         copymode:=cmsrcand;         draw(x,y,g_latar_gajah);
         copymode:=cmsrcpaint;         draw(x,y,g_gajah_putih);
      end;
      if piece=_MENTRIPUTIH then begin
         copymode:=cmsrcand;         draw(x,y,g_latar_mentri);
         copymode:=cmsrcpaint;         draw(x,y,g_mentri_putih);
      end;
      if piece=_MENTRIHITAM then begin
         copymode:=cmsrcand;         draw(x,y,g_latar_mentri);
         copymode:=cmsrcpaint;         draw(x,y,g_mentri_hitam);
      end;
      if piece=_RAJAHITAM then begin
         copymode:=cmsrcand;         draw(x,y,g_latar_raja);
         copymode:=cmsrcpaint;         draw(x,y,g_raja_hitam);
      end;
      if piece=_RAJAPUTIH then begin
         copymode:=cmsrcand; draw(x,y,g_latar_raja);
         copymode:=cmsrcpaint;  draw(x,y,g_raja_putih);
      end;
   end;
//   form1.image2.repaint;
   image1.Canvas.unLock;
end;

procedure tform1.gambar1kotak;
begin
// image1.Canvas.Lock;
 with form1.image1 do begin
   bitblt(form1.image1.canvas.handle,b*_SIZE+_POSKIRI,a*_SIZE+_POSATAS,_SIZE,_SIZE,form1.imageself.canvas.handle,b*_SIZE+_POSKIRI,a*_SIZE+_POSATAS,srccopy);
   if data.papan[map(7-a,b)]=_PIONPUTIH then begin
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_latar_pion.Canvas.handle,0,0,srcand);
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_pion_putih.Canvas.handle,0,0,srcpaint);
   end else
   if data.papan[map(7-a,b)]=_PIONHITAM then begin
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_latar_pion.Canvas.handle,0,0,srcand);
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_pion_hitam.Canvas.handle,0,0,srcpaint);
   end else
   if data.papan[map(7-a,b)]=_BENTENGPUTIH then begin
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_latar_benteng.Canvas.handle,0,0,srcand);
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_benteng_putih.Canvas.handle,0,0,srcpaint);
   end
   else
   if data.papan[map(7-a,b)]=_BENTENGHITAM then begin
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_latar_benteng.Canvas.handle,0,0,srcand);
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_benteng_hitam.Canvas.handle,0,0,srcpaint);
   end
   else
   if data.papan[map(7-a,b)]=_KUDAPUTIH then begin
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_latar_kuda.Canvas.handle,0,0,srcand);
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_kuda_putih.Canvas.handle,0,0,srcpaint);
   end
   else
   if data.papan[map(7-a,b)]=_KUDAHITAM then begin
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_latar_kuda.Canvas.handle,0,0,srcand);
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_kuda_hitam.Canvas.handle,0,0,srcpaint);
   end
   else
   if data.papan[map(7-a,b)]=_GAJAHPUTIH then begin
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_latar_gajah.Canvas.handle,0,0,srcand);
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_gajah_putih.Canvas.handle,0,0,srcpaint);
   end
   else
   if data.papan[map(7-a,b)]=_GAJAHHITAM then begin
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_latar_gajah.Canvas.handle,0,0,srcand);
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_gajah_hitam.Canvas.handle,0,0,srcpaint);
   end
   else
   if data.papan[map(7-a,b)]=_MENTRIPUTIH then begin
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_latar_mentri.Canvas.handle,0,0,srcand);
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_mentri_putih.Canvas.handle,0,0,srcpaint);
   end
   else
   if data.papan[map(7-a,b)]=_MENTRIHITAM then begin
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_latar_mentri.Canvas.handle,0,0,srcand);
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_mentri_hitam.Canvas.handle,0,0,srcpaint);
   end
   else
   if data.papan[map(7-a,b)]=_RAJAPUTIH then begin
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_latar_raja.Canvas.handle,0,0,srcand);
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_raja_putih.Canvas.handle,0,0,srcpaint);
   end
   else
   if data.papan[map(7-a,b)]=_RAJAHITAM then begin
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_latar_raja.Canvas.handle,0,0,srcand);
         bitblt(canvas.handle,b*_SIZE+_POSKIRI+1,a*_SIZE+_POSATAS+1,48,48,g_raja_hitam.Canvas.handle,0,0,srcpaint);
   end
 end;
 //image1.Canvas.unLock;
end;


procedure tform1.gambar(var data:tdata;nodraw:byte);
var a,b:byte;
dest:trect;
begin
//  image1.canvas.Lock;
  for a:=0 to 7 do
   for b:=0 to 7 do
   begin
     if map(7-a,b)<>nodraw then
     begin
       gambar1kotak(data,a,b);
     end else
     begin
         image1.canvas.copymode:=cmsrccopy;
         dest:=rect(b*_SIZE+_POSKIRI,a*_SIZE+_POSATAS,b*_SIZE+_POSKIRI+_SIZE,a*_SIZE+_POSATAS+_SIZE);
    	   image1.canvas.copyrect(dest,form1.imageself.canvas,dest);
     end;
   end;
  form1.image1.repaint;
//  image1.Canvas.Unlock;
end;

procedure tform1.init_board;
begin
  form1.image1.Picture.LoadFromFile(defaultdir+'\board\marbleblue.bmp');
  form1.imageself.Picture.LoadFromFile(defaultdir+'\board\marbleblue.bmp');
end;

procedure loadsetting;
var setting:tsetting;
f:file;
begin
  assignfile(f,form1.defaultdir+'\setting.dat');
  reset(f,1);
  if ioresult=0 then
  begin
    blockread(f,setting,sizeof(setting));
    maxhash:=setting.maxhash;
    futpruningok:=setting.futpruningOK;
    lazyok:=setting.lazyOK;
    qpruningok:=setting.qpruningOK;
    nullok:=setting.nullOK;
    futil_margin:=setting.futil_margin;
    q_margin:=setting.q_margin;
    thread_priority:=setting.thread_priority;
    pushed_pawn_extension_midgame:=setting.PUSHED_PAWN_EXTENSION_MIDGAME;
    pushed_pawn_extension_endgame:=setting.PUSHED_PAWN_EXTENSION_ENDGAME;
    pushed_pawn_extension_late_endgame:=setting.PUSHED_PAWN_EXTENSION_LATE_ENDGAME;
    skak_extension:=setting.SKAK_EXTENSION;
    single_reply_extension:=setting.SINGLE_REPLY_EXTENSION;
    mate_threat_extension:=setting.MATE_THREAT_EXTENSION;
  end;
end;

procedure TForm1.FormCreate(Sender: TObject);
begin
  assignfile(ftext,'debug.txt');
  rewrite(ftext);





  init_var;
  init_image;
  init_board;
  start_new_game;
  addundo(undo,data,clockwhite,clockblack);
  fillchar(historyw,sizeof(historyw),0);
  fillchar(historyb,sizeof(historyb),0);

//  caption:=inttostr(sizeof(book));

  if paramcount>0 then
  begin

    if (pos('xboard',paramstr(1))<>0) then
    begin
        usewinboard:=true;
        start_winboard;
    end;
  end;
//  caption:=inttostr(myrandom);
end;

procedure TForm1.FormActivate(Sender: TObject);
begin
  gambar(data,_NOTMOVEPIECE);
end;

procedure TForm1.image1MouseMove(Sender: TObject; Shift: TShiftState; X,
  Y: Integer);
var source:trect;
begin
  if moving then begin
    source:=rect(0,0,60,60);
    image1.canvas.copymode:=cmsrccopy;
    image1.canvas.copyrect(piece_rect,image2.canvas,source);//mengcopy kotak image.canvas
    image1.canvas.copymode:=cmsrccopy;
    piece_rect:=rect(x-30,y-30,x+30,y+30);
    image2.canvas.copyrect(source,form1.image1.canvas,piece_rect);//mengcopy kotak di sekitar posisi mouse ke image.canvas
    gambar_piece_moving(x-30,y-30,piece_moving);//menggambar biji yang digerakkan
  end;

  x:=(x-_POSKIRI) div _SIZE;
  y:=(y-_POSATAS) div _SIZE;

  if (y in[0..7]) and (x in[0..7]) then
  begin
    if (data.papan[map(7-y,x)]<>0) and (y in[ 0..7]) and (x in [0..7])
        and (((giliran=_SISIPUTIH) and (data.papan[map(7-y,x)]>0))
        or ((giliran=_SISIHITAM) and (data.papan[map(7-y,x)]<0)))
        then
   	      	form1.image1.cursor:=crhandpoint else form1.image1.cursor:=crarrow;
  end else form1.image1.cursor:=crarrow;
//   end;
end;


procedure tform1.make(moves:integer);
var temp:tdata;
pp:boolean;
begin
     temp:=data;
     inc(jumlahlangkah);

     if giliran=_SISIPUTIH then
     begin
       makewhitemove(moves,data);
       if white_checked(data) then
       begin
         data:=temp;
         gambar(data,_NOTMOVEPIECE);
         tulisdiskak(giliran);exit;
       end;
       if checkresult(data) then exit;
     end else
     begin
       makeblackmove(moves,data);
       if black_checked(data) then
       begin
         data:=temp;
         gambar(data,_NOTMOVEPIECE);
         tulisdiskak(giliran);exit;
       end;
       if checkresult(data) then exit;
     end;
     if addrephash(data.hashkey,2-giliran)=2 then
     begin
        form1.statusbar1.Panels[0].Text:='Draw by repetition';
        form1.giliran:=_GAMEOVER;
        form1.gambar(data,200);
        exit;
     end;
     giliran:=3-giliran;
     addundo(undo,data,clockwhite,clockblack);
     tulisgiliran;
     updatetimer(3-giliran);
     stop_ponder;
     if players<>_TWOPLAYERS then
       thr_search:=tsearch.create;

end;

procedure TForm1.Image1MouseUp(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
label finish;
var
temp:tdata;
source:trect;
n,sfrom,sto,moves:integer;
pp:boolean;
begin
 if moving then begin//jika ada biji yang digerakkan

   source:=rect(0,0,60,60);
   form1.image2.canvas.copymode:=cmsrccopy;
   form1.image2.canvas.copyrect(piece_rect,image1.canvas,source);//mengcopy kotak image.canvas
   moving:=false;

   x:=(x-_POSKIRI) div _SIZE;
   y:=(y-_POSATAS) div _SIZE;
   y:=7-y;

   sfrom:=map(y_asal,x_asal);
   sto:=map(y,x);
   moves:=sfrom+sto shl 7;

   if is_move_legal(moves,giliran) then
   begin
     make(moves);
   end;

   gambar(data,_NOTMOVEPIECE);
 end;
end;


procedure TForm1.New1Click(Sender: TObject);
begin
  if formnewgame.showmodal=mrok then
  begin
    stop_thinking;
//    thr_search.Free;
    start_new_game;
    form1.StatusBar1.Panels[0].text:='';
    if players=_COMPVSPLAYER then
      thr_search:=tsearch.create;
    forminfo.writeinfo;
    if usetimer then
    begin
      clockblack:=realclock;
      clockwhite:=realclock;
    end else
    begin
      clockblack:=0;clockwhite:=0;
    end;
  end;
end;

procedure TForm1.Perft1Click(Sender: TObject);
var
ls:string;
l:integer;
begin
 ls:=inputbox('Perft','Masukkan level: ','');
 if ls<>'' then
 begin
   l:=strtoint(ls);
   perft_depth:=l;
   thr_perft:=tperft.create;
 end;
{    if anotherdraw(data,form1.giliran)
    or anotherdraw2(data,form1.giliran)
    then
    begin
      caption:='draw';
    end;
}
end;

constructor tsearch.create;
begin
  freeonterminate:=true;
  onterminate:=terminated;
  inherited create(false);
end;

procedure savesetting;
var setting:tsetting;
f:file;
begin
end;

procedure TForm1.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  freeimage;
  savesetting;
end;

procedure savefile(filename:string);
var sav:tsav;
f:file;
begin
     sav.data:=data;
     sav.giliran:=form1.giliran;
     sav.players:=players;
     sav.usetimer:=usetimer;
     sav.clockputih:=clockwhite;
     sav.clockhitam:=clockblack;
     sav.undo:=undo;
     form1.lastsavedfile:=FileName;
     assignfile(f,filename);
     rewrite(f,1);
     blockwrite(f,sav,sizeof(sav));
     closefile(f);
     form1.tulisgiliran;
end;

procedure TForm1.Saveas1Click(Sender: TObject);
begin
  if savedialog1.Execute then
  begin
    savefile(savedialog1.filename);
  end;
end;

procedure TForm1.Load1Click(Sender: TObject);
var sav:tsav;
f:file;
begin
  if opendialog1.execute then
  begin
    delete_data;
    assignfile(f,opendialog1.filename);
    reset(f,1);
    blockread(f,sav,sizeof(sav));
    data:=sav.data;
    giliran:=sav.giliran;
    players:=sav.players;
    usetimer:=sav.usetimer;
    undo:=sav.undo;
    clockwhite:=sav.clockputih;
    clockblack:=sav.clockhitam;
    gambar(data,200);
    initrepdata;
    tulisgiliran;
  end;
end;

procedure TForm1.TimerMSGTimer(Sender: TObject);
begin
  inc(intvmsg);
  if intvmsg>2 then
  begin
    statusbar1.Panels[0].Text:='';
    timermsg.Enabled:=false;
  end;
end;

procedure TForm1.N41Click(Sender: TObject);
begin
 (sender as tmenuitem).Checked:=true;
 fixeddepth1.Checked:=true;
 toplevel:=(sender as tmenuitem).Tag;
 tulislevel;
 usetimer:=false;
// clockwhite:=0;clockblack:=0;
end;

procedure TForm1.Custom2Click(Sender: TObject);
var s:string;
begin
  s:=inputbox('Custom Ply','Masukkan ply: ','');
  if s<>'' then
  begin
    custom2.checked:=true;
    fixeddepth1.Checked:=true;
    toplevel:=strtoint(s);
    usetimer:=false;
    tulislevel;
  end;
end;

procedure TForm1.N5mingame1Click(Sender: TObject);
var a:integer;st,st2:string;
begin
 (sender as tmenuitem).Checked:=true;
 imer1.checked:=true;
 clockwhite:=(sender as tmenuitem).tag*timefactor*60;
 clockblack:=clockwhite;
 realclock:=clockwhite;
 usetimer:=true;
 formclock.showclock;
 st:=(sender as tmenuitem).Caption;st2:='';
 for a:=1 to length(st) do
   if st[a] <>'&' then st2:=st2+st[a];
 statusbar1.panels[2].text:=st2;
 modetime:=MINUTES_GAME;
end;

procedure TForm1.custom1Click(Sender: TObject);
var s:string;
begin
  if form10.showmodal=mrok then
  begin
   usetimer:=true;
   (sender as tmenuitem).Checked:=true;
   imer1.checked:=true;
   //clockwhite:=(sender as tmenuitem).tag*timefactor*60;
   formclock.showclock;
   statusbar1.Panels[2].Text:=form10.stoutput;
   n5mingame1.Checked:=false;
   n15mingame1.Checked:=false;
   n60mingame1.Checked:=false;
  end;
end;

procedure TForm1.EditPosition1Click(Sender: TObject);
begin
 formsetupboard1:=tformsetupboard1.Create(self);
 if formsetupboard1.showmodal=mrok then
 begin
  gambar(data,200);
  initrepdata;
  delete_data;
  tulisgiliran;
//  resetundo(undo);
{  if ((players=_COMPVSPLAYER) and (giliran=_SISIPUTIH))
  or ((players=_PLAYERVSCOMP) and (giliran=_SISIHITAM))
  then
    thr_search:=tsearch.create;}
 end;

end;

procedure TForm1.Evaluate1Click(Sender: TObject);
var f,e:integer;
b:boolean;
begin
  f:=giliran;
  giliran:=_SISIPUTIH;
  e:=eval(data,giliran,-_INFINITY,_INFINITY,false);
  form3.Memo1.Lines.clear;
  form3.Memo1.Lines.add('Nilai eval = '+inttostr(e));
  form3.Memo1.Lines.add('Nilai material = '+inttostr(materialvalue(giliran,data.materialscore)));
  form3.Memo1.Lines.add('Pawn shelter eval = '+inttostr(kseval));
  form3.Memo1.Lines.add('Pawn eval           = '+inttostr(pawneval));
  form3.Memo1.Lines.add('Knight eval         = '+inttostr(knighteval));
  form3.Memo1.Lines.add('Bishop eval         = '+inttostr(bishopeval));
  form3.Memo1.Lines.add('Rook eval           = '+inttostr(rookeval));
  form3.Memo1.Lines.add('Queen eval          = '+inttostr(queeneval));
  form3.Memo1.Lines.add('King pressure eval  = '+inttostr(kpeval));
  form3.Memo1.Lines.add('Tropism       eval  = '+inttostr(tropismeval));
  giliran:=f;
end;

procedure TForm1.BenchMark1Click(Sender: TObject);
begin
  if formalgoritma.showmodal=MROK then
  begin
//    inittable;
  end;
  forminfo.writeinfo;
end;

procedure TForm1.Switchside1Click(Sender: TObject);
begin
{ if players=_COMPVSPLAYER then
 begin
   players:=_PLAYERVSCOMP;
   thr_search:=tsearch.create;
 end
 else if players=_PLAYERVSCOMP then
 begin
   players:=_COMPVSPLAYER;
   thr_search:=tsearch.create;
 end;
}
 if computer_thinking=false then
   thr_search:=tsearch.create;
end;

procedure TForm1.SearchStatistic1Click(Sender: TObject);
begin
  SearchStatistic1.Checked:=not SearchStatistic1.checked;
  if SearchStatistic1.Checked then
  begin
    form3.show;
  end else
  begin
    form3.Hide;
  end;
end;

procedure arrangelayout;
begin
 formclock.top:=1;
 formclock.left:=form1.Left+form1.Width+0;
 form3.left:=form1.Left+form1.Width+0;
 form3.top:=formclock.top+formclock.Height+0;
 form3.Width:=forminfo.Width;
 formiterative.Left:=form1.left+form1.width+0;
 formiterative.top:=form3.top+form3.Height+0;

 forminfo.left:=form1.Left+form1.Width+0;
 forminfo.top:=formiterative.top+formiterative.Height+0;
end;

procedure TForm1.FormShow(Sender: TObject);
begin
 arrangelayout;
 if not usewinboard then
 begin
   form3.show;
   formInfo.show;
   formiterative.show;
   formclock.show;
 end;

end;

procedure TForm1.About1Click(Sender: TObject);
begin
  formabout.showmodal;
end;

procedure TForm1.ChangePlayer1Click(Sender: TObject);
//var temp:tdata;
begin
//  temp:=data;
  if formnewgame.showmodal=mrok then
  begin
//    data:=temp;
    if (players=_PLAYERVSCOMP) and (giliran=_SISIHITAM) then
      thr_search:=tsearch.create;
    if (players=_COMPVSPLAYER) and (giliran=_SISIPUTIH) then
      thr_search:=tsearch.create;
    forminfo.writeinfo;
//    gambar(data,200);
  end;
end;

procedure TForm1.ToolButton9Click(Sender: TObject);
begin
  killperftthread;
end;

procedure TForm1.Information1Click(Sender: TObject);
begin
  Information1.Checked:=not Information1.checked;
  if Information1.Checked then
  begin
    forminfo.show;
  end else
  begin
    forminfo.Hide;
  end;
end;

procedure TForm1.Timer1Timer(Sender: TObject);
begin
 if not usetimer then
 begin
   if giliran=_SISIPUTIH then inc(clockwhite)
   else inc(clockblack);
   formclock.showclock;
 end else
 begin
   if giliran=_SISIPUTIH then dec(clockwhite)
   else dec(clockblack);
   if not usewinboard then
     formclock.showclock;
   inc(timepassed);
 end;
end;

procedure TForm1.Clock1Click(Sender: TObject);
begin
  clock1.Checked:=not clock1.checked;
  if clock1.Checked then
  begin
    formclock.show;
  end else
  begin
    formclock.Hide;
  end;

end;

procedure TForm1.Iterative1Click(Sender: TObject);
begin
  iterative1.Checked:=not iterative1.checked;
  if iterative1.Checked then
  begin
    formiterative.show;
  end else
  begin
    formiterative.Hide;
  end;

end;

procedure TForm1.Undo1Click(Sender: TObject);
begin
 if takeback(undo,data,clockwhite,clockblack,giliran) then
 begin
   stop_thinking;
   gambar(data,200);
   delete_data;
   dec(jumlahlangkah);
   initrepdata;
   tulisgiliran;
 end;
end;

procedure TForm1.Redo1Click(Sender: TObject);
begin
  if moveforward(undo,data,clockwhite,clockblack,giliran) then
  begin
    stop_thinking;
    gambar(data,200);
    delete_data;
    inc(jumlahlangkah);
    initrepdata;
    tulisgiliran;
  end;
end;

procedure TForm1.Save1Click(Sender: TObject);
begin
  if lastsavedfile<>''then
    savefile(lastsavedfile)
  else
    saveas1click(sender);
end;

procedure TForm1.InputMove1Click(Sender: TObject);
var s:string;
move:integer;
pp:boolean;
f9:tform9;
begin
  f9:=tform9.Create(self);
  if f9.showmodal=mrok then
  begin
    s:=f9.output;
    if s<>'' then
    begin
      move:=notationtomove(s,data,giliran);
      if isvalid(data,move,giliran) then
      begin
        make(move);
        gambar(data,200);
      end else showmessage('Invalid move');
    end;
  end;
end;

procedure TForm1.LearnPGN1Click(Sender: TObject);
begin
  if opendialog2.Execute then
  begin
    learn_pgn(opendialog2.FileName);
  end;
end;

procedure TForm1.EvasionTest1Click(Sender: TObject);
begin
  if form11.showmodal=mrok then
  begin
    modetime:=form11.testtype;
    runningtest:=true;
    if modetime=FIXED_DEPTH then
    begin
      toplevel:=form11.testlength;
      usetimer:=false;
    end
    else
    begin
      realclock:=form11.testlength;
      clockwhite:=realclock;
      clockblack:=realclock;
      usetimer:=true;
    end;
    form11.testfile:=defaultdir+form11.testfile;
    if form11.ComboBox1.ItemIndex=0 then
      form1.StatusBar1.panels[3].Text:='Running ECM TEST'
    else
      form1.StatusBar1.panels[3].Text:='Running WAC TEST';
    start_test(form11.testfile);
  end;
end;

procedure TForm1.FormPaint(Sender: TObject);
begin
// if usewinboard then form1.Hide
end;

procedure TForm1.Evasion1Click(Sender: TObject);
var ml:tmovelist;
jml,a:integer;
s:string;
begin
end;

procedure TForm1.ForceMove1Click(Sender: TObject);
begin
 stop_thinking;
end;

procedure TForm1.Editor1Click(Sender: TObject);
begin
  form12.showmodal;
end;

procedure TForm1.FormDestroy(Sender: TObject);
begin
  closefile(ftext);
end;

procedure TForm1.FEN1Click(Sender: TObject);
begin
  clipboard.astext:=GetFEN(data,giliran);
end;

end.

