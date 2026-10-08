unit search;
{$DEFINE USEFUTIL}


interface
uses header;

var curr_move:array[0..63] of integer;
{$IFDEF USEFUTIL}
Const
  Min_Try=50*6;
  Min_Threshold=0.10;
Const
  NODE_ALL=-1;
  NODE_PV=0;
  NODE_CUT=1;
var
hist_tried,hist_Succ:array[-6..6,0..63] of integer;
stack:array[0..63] of record
  fase,legalmove:integer;
  mte,ext:boolean;
  evaluated:boolean;EvalValue:integer;
end;

Procedure empty_HistoryTable;
{$ENDIF}


function mainsearch(level,giliran,alpha,beta:integer;var data:tdata;nullmove:boolean;depth:integer;cext:integer;nmtree:boolean;reduced,nodetype:integer):integer;
function Q_search(giliran,alpha,beta:integer;var data:tdata;ply:integer;depth,cext,neval:integer;nodetype:integer):integer;
//function aspiration(level,giliran,tipe:integer;data:tdata):integer;
var rs:integer;
{$IFDEF stat}
var fl,fh,pvn:integer;

{$ENDIF}
implementation
uses ueval,makemove,tools,hashing,next,movgen,bitboard_mask,usee,repetition;
const margin:array[2..8] of integer=(16,22,30,37,55,92,142);
margingc:array[0..15] of integer=(5000,90,50,10,10,0,0,0,0,0,0,0,0,0,0,0);

const noval=-200000;
{$IFDEF USEFUTIL}
Procedure empty_HistoryTable;
begin
  fillchar(hist_tried,sizeof(hist_tried),0);
  fillchar(hist_succ,sizeof(hist_succ),0);
end;
{$ENDIF}

procedure addhistory(rlevel,depth,moves,giliran,piece:integer);
begin
  if giliran=_SISIPUTIH then
  begin
//    assert(data.papan[moves shr 7 and 127]>0);
    inc(historyw[piece,moves shr 7 and 127],(rlevel) * (rlevel) )
  end
  else
  begin
//    assert(data.papan[moves shr 7 and 127]<0);
    inc(historyb[piece,moves shr 7 and 127],(rlevel) * (rlevel) );
  end;
  if (killer0[depth]<>moves) then
  begin
    killer1[depth]:=killer0[depth];
    killer0[depth]:=moves;
  end;

end;


function mainsearch;
var g,fase:integer;
temp:tdata;
ml,mlcap:tmovelist;
var curmovecap,curmovenoncap,jmlcap,jmlnoncap,newlevel,nlev,jml:integer;
tcext,t_alpha,t_beta,nilai,tempi,areduce:integer;
sto:byte;
nullmoveok,incheck,mte:boolean;
h:int64;
goodcap,tempep:integer;
r,stat:byte;
rlevel,legalmove,moves,hmoves,tmoves:integer;
neval:integer;

futflag:boolean;

//var tm:array[0..65535] of boolean;
label search1,search2,Nonullmove,search3;
{$IFDEF USEFUTIL}
Function Get_moving_piece:shortint;
begin
//  result:=0;
  result:=temp.papan[moves and 127];
end;

Function Get_Dest(sto:integer):shortint;
begin
  result:=-1;
  case sto of
    _ROKADEPANJANG : if giliran=_SISIPUTIH then result:=c1 else result:=c8;
    _ROKADEPENDEK : if giliran=_SISIPUTIH then result:=g1 else result:=g8;
    else result:=sto;
  end;
end;
{$ENDIF}
function seval:Integer;
begin
    if (neval=noval) and (depth>=2) then
      result:=eval(data,giliran,-_INFINITY,_INFINITY)
    else
      result:=neval;
end;

var scoreDraw,isScoreDraw:boolean;
donotreduce:Boolean;
begin
    if (timepassed>=timelimit) then exit;
    if stop_process then exit;

    stack[depth].Ext:=false;
    if data.move50count>=101 then
    begin
      result:=CONTEMP_DRAW;exit;
    end;
    hmoves:=_NO_MOVE;
    futflag:=false;
    if searchhash(data.hashkey,alpha,beta,level,giliran,nilai,hmoves,futflag)
//    and
    and (nodetype<>NODE_PV)
    then
    begin
    {$IFDEF stat}
      inc(hashco);
    {$ENDIF}
      result:=nilai;exit;

    end;
//    fillchar(tm,sizeof(tm),false);
    h:=data.hashkey;
    rlevel:=level shr 3;

    mte:=false;
{    if depth>=maxdepth then
       ext:=false;
}
    if giliran=_SISIPUTIH then
    begin
      if white_checked(data) then
      begin
        incheck:=true;
        stack[depth].Ext:=true;
        if cext>0 then
        begin
          if (data.nilai_perwira_putih+data.nilai_perwira_hitam>24)
          or (alpha<>beta-1)
          then
          begin
            inc(level,6);
          end
          else
            inc(level,2);
        end;

        goto nonullmove;
      end
      else
        incheck:=false;
    end else
    begin
      if black_checked(data) then
      begin
        incheck:=true;
        stack[depth].Ext:=true;
        if cext>0 then
        begin
          if (data.nilai_perwira_putih+data.nilai_perwira_hitam>24)
          or (alpha<>beta-1)
          then
          begin
            inc(level,6);

          end
          else
            inc(level,2);
        end;

        goto nonullmove;
      end else
        incheck:=false;
    end;
    stack[depth].mte:=false;
    if (rlevel>=2) and (beta<_INFINITY-100) and not nullmove
    and (materialvalue(giliran,data.materialscore)>=-500) and (alpha=beta-1)
    and not futflag
    //and (eval(data,giliran,-_INFINITY,_INFINITY,false)>=beta)
    //and (seval>=beta)
    then
    begin
      if giliran=_SISIPUTIH then
      begin
        tempep:=data.nilai_perwira_putih;
      end
      else
      begin
        tempep:=data.nilai_perwira_hitam;
      end;
      if tempep>=3 then
      begin
        tempep:=data.ep;
        case rlevel of
          4 : begin
            if materialvalue(giliran,data.materialscore)>=alpha+500 then
            newlevel:=level-32
            else
            begin
            if nodetype=NODE_PV then
              newlevel:=level-24
            else
              newlevel:=level-32;
            end;
          end;
          else newlevel:=level-32;
        end;
//        newlevel:=level-FRACTIONAL_PLY-r;
        if data.ep<>_NO_EP then
        begin
          data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
          data.ep:=_NO_EP;
          data.hashkey:=data.hashkey xor enpassanthashkey[_NO_EP];
        end;
        areduce:=0;
        stack[depth].evaluated:=false;
        if newlevel<FRACTIONAL_PLY then
        begin
          if (data.nilai_perwira_putih<=9) and (data.nilai_perwira_hitam<=9) then
            nilai:=-q_search(3-giliran,-beta,-(beta-1),data,0,depth+1,cext,noval,-nodetype)
          else
            nilai:=-q_search(3-giliran,-beta,-(beta-1),data,1,depth+1,cext,noval,-nodetype);
        end
        else
          nilai:=-mainsearch(newlevel,3-giliran,-beta,-(beta-1),data,true,depth+1,cext,true,0,-nodetype);
        if nilai>=beta then
        begin
          {if (level>=64) and (material(data,giliran)<=12) then
          begin
            if mainsearch(level-56,giliran,alpha,beta,data,true,depth,cext,true,0,-nodetype)>=beta
            then
            begin
              data.hashkey:=h;
              data.ep:=tempep;
              result:=nilai;exit;
            end;
          end else}
          begin
            data.hashkey:=h;
            data.ep:=tempep;
            result:=nilai;exit;
          end;
        end;
        if (nilai=-_NILAI_RAJA+depth+2) and (cext>0) then
        begin
          stack[depth].mte:=true;
          inc(level,6);
          dec(cext,6);
          mte:=true;
        end;
        data.hashkey:=h;
        data.ep:=tempep;
      end;
    end;
nonullmove:
    donotreduce:=false;
    if (stack[depth-1].evaluated)
    and (
      (stack[depth-1].evalValue-materialValue(giliran,data.materialscore)>=90)
      or (stack[depth-1].evalValue>=beta))
    then
      donotreduce:=true;
    fase:=0;
    temp:=data;
    curmovecap:=0;
    tmoves:=_NO_MOVE;
    t_alpha:=alpha;t_beta:=beta;
    g:=-_INFINITY-100;
    legalmove:=0;
    nlev:=level-FRACTIONAL_PLY;
    tcext:=cext;
    goodcap:=0;
    rlevel:=level shr 3;
    IsScoreDraw:=false;
//    jml:=0;

    repeat
      stack[depth].evaluated:=false;
      scoreDraw:=false;
      futflag:=false;

      neval:=noval;
      cext:=tcext;
      newlevel:=nlev;
      areduce:=0;
{      inc(anu7);
      if anu7=370 then
        inc(anu7);}
      if not incheck then
      begin
        {jika tidak diskak sebelum melangkah, maka panggil nextmove}
        nextmove(giliran,depth,data,moves,fase,hmoves,curmovecap,curmovenoncap,jmlcap,jmlnoncap,ml,mlcap,goodcap);
        if moves=_NO_MOVE then
           break;

        sto:=moves shr 7 and 127;
        if giliran=_SISIPUTIH then
        begin
          {karena tidak diskak sebelum melangkah, maka hanya dua kemungkinan pihak yang melangkah
          diskak setelah melangkah, yaitu jika melangkahkan raja ke petak yang diserang buah lawan
          atau karena open skak}
          if (data.papan[moves and 127]=_RAJAPUTIH) then
          begin
            {melangkahkan raja, maka cek apakah petak tujuan diserang oleh buah lawan}
            if white_attacked(data,sto) then
            begin
              continue;
            end;
          end else
          {jika bukan langkah raja, maka cek apakah langkah yang dilakukan menyebabkan open skak}
          if white_open_check(data,moves) then
          begin
            continue;
          end;
        end else
        begin
          if (data.papan[moves and 127]=_RAJAHITAM) then
          begin
            if black_attacked(data,sto) then
            begin
              continue
            end;
          end else
          if black_open_check(data,moves) then
          begin
            continue;
          end;
        end;
      end else
      begin
        {jika diskak sebelum melangkah, maka panggil nextevasion untuk meng-generate hanya langkah
         yang lolos dari skak}
        if nextevasion(giliran,data,moves,fase,hmoves,curmovecap,jml,jmlcap,ml)
        then begin
          result:=-_NILAI_RAJA+depth;
          exit;
        end;
        if moves=_NO_MOVE then
        begin
           break;
        end;
        sto:=moves shr 7 and 127;

        if (legalmove=0) and (cext>0) //and not losingmaterial(data.materialscore,giliran)
        then
        begin
          {jumlah langkah yang bisa dilakukan hanya satu, maka extend}
          if jml = 1 then
            begin
              inc(newlevel,SINGLE_REPLY_EXTENSION);
              inc(nlev,SINGLE_REPLY_EXTENSION);
              inc(cext,8);inc(tcext,8);
              dec(newlevel,newlevel and 7);
            end;
          {double skak biasanya lebih berbahaya daripada skak tunggal,
          maka extensionnya ditambah sedikit}
          if jmlcap=2 then
          begin
            inc(newlevel,DOUBLE_SKAK_EXTENSION);
            inc(nlev,DOUBLE_SKAK_EXTENSION);
            dec(cext,DOUBLE_SKAK_EXTENSION);dec(tcext,DOUBLE_SKAK_EXTENSION);
          end;
          {Untuk peningkatakan kecepatan,
          Skak extension tidak dilakukan pada saat menskak lawan,
          tapi dlakukan pada saat akan menghindar dari skak}

          dec(cext,SKAK_EXTENSION);
          dec(tcext,SKAK_EXTENSION);

        end;
      end;

      if giliran=_SISIPUTIH then
        makewhitemove(moves,data)
      else
        makeblackmove(moves,data);

      if sto=_EN_PASSANT then
      begin
        if ((giliran=_SISIPUTIH) and white_checked(data))
        or ((giliran=_SISIHITAM) and black_checked(data)) then
        begin
          data:=temp;
//          dec(total_node);
          continue;
        end;
      end;

{$IFDEF USEFUTIL}
      if data.cp=0 then
        inc(hist_tried[get_moving_piece,get_dest(sto)],rlevel*rlevel);
{$ENDIF}

      curr_move[depth]:=moves;

      inc(legalmove);

      {masukkan posisi dalam sekunder hash key untuk pengecekan draw repetition}
      {tidak perlu menunggu sampai tiga kali, jika posisi yang sama telah terja dua kali
      dalam sebuah path, maka sudah dapat dianggap remis}
      if addrephash(data.hashkey,2-giliran)=1 then
      begin
        nilai:=contemp_draw;
        ScoreDraw:=true;
        goto search3;
      end;
      stack[depth].fase:=fase;
      stack[depth].legalmove:=legalmove;



      if (data.nilai_perwira_putih=0) and (data.nilai_perwira_hitam=0)
      and (nodetype=NODE_PV)
      then
      begin
        if (data.cp=1) and (cext>0) then
        begin
          inc(newlevel,4);
          dec(cext,4);
        end;
        if (data.cp>=_KUDAPUTIH) and (data.cp<=_MENTRIPUTIH) { and (data.pawnending=false)}
        and (data.materialscore>=-385) and (data.materialscore<=385) and (cext>0)
        then
        begin
          dec(cext,16);
          inc(newlevel,16);
          goto search1;
        end;
      end;
      {cek apakah bisa melakukan pawn push extension}
      if (sto<=63) and (abs(data.papan[sto])=_PIONPUTIH)
      and (nodetype=NODE_PV)
      and (cext>0)
      and
      (
      ((giliran=_SISIPUTIH) and (sto>=a7))
      or
      ((giliran=_SISIHITAM) and (sto<=h2))
      )
      and not incheck
      and (see(giliran,temp,moves and 127,sto)>=0)
//      and IsExtension(data,newlevel,giliran,moves,cext)
      //and not winningmaterial(data.materialscore,giliran)
      then
      begin
        inc(newlevel,8);
        dec(cext,8);
        goto search1;
      end;

      if
      (alpha<>beta-1)
      //(nodetype=NODE_PV)
      and (data.cp>1)
      and (data.cp<9)
      and (data.cp=temp.cp)
      and not incheck
      and (cext>0)
      and not mte
      then
      begin
        inc(newlevel,8);
        dec(cext,8);
        goto search1;
      end;


      if (rlevel<=2) {and ((data.cp=0)) }and not incheck and (data.cp=0) and (beta-alpha=1)
      and not stack[depth-1].mte
      and not stack[depth-1].Ext
      //and (alpha>-700)
      and (nodetype<>NODE_PV)
      //and (fase>_KILLERMOVES3)
      and (moves<>hmoves)
      then
      begin
        case rlevel of
        1:
          begin
              tempi:=materialvalue(giliran,data.materialscore){+captured_value(sto,temp)};
              if (data.nilai_perwira_putih+data.nilai_perwira_hitam<=24) then
                inc(tempi,300);
              if ((tempi+250<t_alpha) )
              and not menskak2(temp,giliran,moves)
              and (data.nilai_perwira_putih>3) and (data.nilai_perwira_hitam>3)
              and not pawn_7_rank(giliran,data)
              and (data.pawnwhite<>0) and (data.pawnblack<>0)
              then
              begin
{                futflag:=true;
                inc(futTry);}
                nilai:=tempi;
                goto search3;
              end;
          end;
          2:
          begin
              tempi:=materialvalue(giliran,data.materialscore){+captured_value(sto,temp)};
              if (data.nilai_perwira_putih+data.nilai_perwira_hitam<=24) then
                inc(tempi,300);
              if ((tempi+300<t_alpha) )
              and not menskak2(temp,giliran,moves)
              and (data.nilai_perwira_putih>5) and (data.nilai_perwira_hitam>5)
              and not pawn_7_rank(giliran,data)
              and not pawn_6_rank(giliran,data)
              and (data.pawnwhite<>0) and (data.pawnblack<>0)
              then
              begin
                begin
{                  futflag:=true;
                  inc(futTry);}
                  nilai:=tempi;
                  goto search3;
                end;
              end;
          end;
          {300:
          begin
              tempi:=materialvalue(giliran,data.materialscore);
              if ((tempi+550<t_alpha) )
              and not menskak2(temp,giliran,moves)
              and (data.nilai_perwira_putih>8) and (data.nilai_perwira_hitam>8)
              and not pawn_7_rank(giliran,data)
              and not pawn_6_rank(giliran,data)
              AND (data.nilai_perwira_putih>0) and (data.nilai_perwira_hitam>0)
              then
              begin
                nilai:=tempi+550;
                goto search3;
              end;
          end;}
        end;
      end;

{
search reduction berdasarkan nilai evaluator
dasar logikanya : jika nilai sebelum lawan melangkah setelah ditambah margin
lebih kecil dari alpha, maka lawan melangkah akan memperburuk nilai itu sehingga
makin lebih kecil dari alpha
}



      if giliran=_SISIPUTIH then
      begin
        if not incheck {and (rlevel<=8) }and (rlevel>=2) and (depth>=2)
        and ((data.cp=0))
        and (fase>_KILLERMOVES3)
        and (alpha=T_alpha)
        and (beta-alpha=1)
        //and not nmtree
        and not pawn_7_rank(_SISIPUTIH,data)
        and not mte
        and not good_move_white(data,sto)

        //and (moves<>hmoves)
        and
        (
        ((data.nilai_perwira_putih>=6) and (data.nilai_perwira_hitam>=6))
        or
        ((data.nilai_perwira_putih>=3) and (data.nilai_perwira_hitam>=3) and (rlevel=2))
        )

//        and (data.materialscore-300<alpha)
        and not white_threat(data,moves,tempi)
        and not black_open_check(temp,moves)
        then
        begin
          if ((tempi=0) or
          (
            (legalmove>5)
            and (stack[depth-2].fase>_KILLERMOVES4)
//            and (stack[depth-2].legalmove>3)
            and (stack[depth-1].legalmove<=3)
            and not stack[depth-1].mte
            //and not stack[depth-2].mte
          ))
          and
          (legalmove>3) and
          //(nilai<t_alpha) and
          (hist_tried[get_moving_piece,get_dest(sto)]>=Min_TRY)
          and
          (hist_succ[get_moving_piece,get_dest(sto)] <= hist_tried[get_moving_piece,get_dest(sto)] shr 3) then
          begin
            dec(newlevel,8);
            areduce:=8;
            if  (hist_succ[get_moving_piece,get_dest(sto)] <= hist_tried[get_moving_piece,get_dest(sto)] shr 4) then
            begin
              dec(newlevel,8);
              areduce:=16;
            end;
            goto search1;
          end;
          if rlevel<=8 then
          begin
          nilai:=eval(data,giliran,-_INFINITY,_INFINITY,true);
          neval:=-nilai;
          stack[depth].evaluated:=true;
          stack[depth].evalValue:=neval;
          if (nilai+tempi+margin[rlevel]+30<t_alpha)
          and not pawn_6_rank(giliran,data)
           then
          begin
            dec(newlevel,24);
            areduce:=24;
          end else
          if nilai+tempi+margin[rlevel]<t_alpha then
          begin
            dec(newlevel,16);areduce:=16;
          end else
          if nilai+tempi+margin[rlevel]-5<t_alpha then
          begin
            dec(newlevel,8);areduce:=8;
          end else
          if (tempi>0) and (nilai+margin[rlevel]+40+tempi shr 1 <t_alpha) then
          begin
{            dec(newlevel,6);
            areduce:=6;}
            newlevel:=0;
          end
          {$IFDEF USEFUTIL}
//          else if
          {$ENDIF}
          else
          if (goodcap>1) and (tempi=0) and (nilai<t_alpha) then
          begin
            if goodcap>=5 then
            begin
              dec(newlevel,16);areduce:=16;
            end else
            begin
              dec(newlevel,8);areduce:=8;
            end;
          end;
          end;
          ;

          //if (newlevel<Fractional_ply) then goto search2;
        end;
      end else
      begin
        if not incheck {and (rlevel<=8)} and (rlevel>=2) and (depth>=2)
        and ((data.cp=0))
        and (fase>_KILLERMOVES3)
        and (alpha=T_alpha)
        and (beta-alpha=1)
        //and not nmtree
        and not pawn_7_rankb(data)
        //and (moves<>hmoves)
        and not mte
        and not good_move_black(data,sto)
        and
        (
        ((data.nilai_perwira_putih>=6) and (data.nilai_perwira_hitam>=6))
        or
        ((data.nilai_perwira_putih>=3) and (data.nilai_perwira_hitam>=3) and (rlevel=2))
        )

//        and (data.materialscore-300<alpha)
        and not black_threat(data,moves,tempi)
        and not white_open_check(temp,moves)   then
        begin
          if
          ((tempi=0)
          or
          (
            (legalmove>5)
            and (stack[depth-2].fase>_KILLERMOVES4)
//            and (stack[depth-2].legalmove>3)
            and (stack[depth-1].legalmove<=3)
            and not stack[depth-1].mte
//            and not stack[depth-2].mte
          ))
          and
          (legalmove>3) and
          //(nilai<t_alpha) and
          (hist_tried[get_moving_piece,get_dest(sto)]>=Min_TRY)
          and
          (hist_succ[get_moving_piece,get_dest(sto)] <= hist_tried[get_moving_piece,get_dest(sto)] shr 3) then
          begin
            dec(newlevel,8);
            areduce:=8;
            if (hist_succ[get_moving_piece,get_dest(sto)] <= hist_tried[get_moving_piece,get_dest(sto)] shr 4) then
            begin
              dec(newlevel,8);
              areduce:=16;
            end;
            goto search1;
          end;
          if rlevel<=8 then
          begin
          nilai:=eval(data,giliran,-_INFINITY,_INFINITY,true);
          neval:=-nilai;
          stack[depth].evaluated:=true;
          stack[depth].evalValue:=neval;
          if (nilai+tempi+margin[rlevel]+30<t_alpha)
          and not pawn_6_rank(giliran,data)
           then
          begin
            dec(newlevel,24);
            areduce:=24;
          end else
          if nilai+tempi+margin[rlevel]<t_alpha then
          begin
            dec(newlevel,16);
            areduce:=16;
          end else
          if nilai+tempi+margin[rlevel]-5<t_alpha then
          begin
            dec(newlevel,8);
            areduce:=8;
          end else
          if (tempi>0) and (nilai+margin[rlevel]+40+tempi shr 1 <t_alpha) then
          begin
            newlevel:=0;
            //areduce:=6;
          end
          {$IFDEF USEFUTIL}
//          else if
          {$ENDIF}
           else
          if (goodcap>1) and (tempi=0) and (nilai<t_alpha) then
          begin
            if goodcap>=5 then
            begin
              dec(newlevel,16);areduce:=16;
            end else
            begin
              dec(newlevel,8);areduce:=8;
            end;
          end;
          end;

          ;


          //if (newlevel<Fractional_ply) then goto search2;
        end;
      end;

      if not incheck and (depth<=10) //and (nodetype<>NODE_PV)
      //and (beta-alpha=1)
      and (data.cp=0)
      and (depth>=2)
      and (legalmove>6)
      and not mte
      and (areduce=0)
      and not stack[depth-1].mte
      and not stack[depth-2].mte
      and not pawn_7_rank(1,data)
      and not pawn_7_rank(2,data)
      and not donotreduce
      and not good_move(data,sto,giliran)
      //and not nullmove
//      and
//      (hist_tried[get_moving_piece,get_dest(sto)]>=Min_TRY)
//      and
//      (hist_succ[get_moving_piece,get_dest(sto)] <= hist_tried[get_moving_piece,get_dest(sto)] shr 6)
      and not menskak2(temp,giliran,moves)
      then
      begin
        //newlevel:=0;
        dec(newlevel,8);
        areduce:=8;
        goto search1;
        {if (depth<=4) and (data.nilai_perwira_putih+data.nilai_perwira_hitam>=24) and not pawn_6_rank(giliran,data) and
         (legalmove>20) and ((hist_succ[get_moving_piece,get_dest(sto)] < hist_tried[get_moving_piece,get_dest(sto)] shr 10)) then
        begin
          nilai:=g;
          goto search3;
        end else
          goto search1;}
        //
      end;
      if not incheck and (rlevel>=4) and (depth>=2)
      and (nodetype<>NODE_PV)
      //and (beta-alpha=1)
      and
      (
      ((nodetype=NODE_CUT) and (legalmove>3))
      or
      ((nodetype=NODE_ALL) and (legalmove>6))
      )
      and not mte
      and (fase>_KILLERMOVES3)
      and (areduce=0)
      and (data.cp=0)
      and not menskak2(temp,giliran,moves)
      and not pawn_7_rank(giliran,data)
      and not good_move(data,sto,giliran)
      and not donotreduce
      then
      begin
        dec(newlevel,8);
        areduce:=8;
      end;

search1:
      if (newlevel<8)  then
      begin
        if (giliran=_SISIPUTIH) then
        begin
          if white_forkandpin(sto,data) and (cext>0) then
          begin

            begin
              newlevel:=8;cext:=0;
            end;

          end;
        end else
        begin
          if black_forkandpin(sto,data) and (cext>0) then
          begin
            begin
              newlevel:=8;cext:=0;
            end;

          end;
        end;
      end;


      if newlevel<FRACTIONAL_PLY then
      begin
        if legalmove=1 then
          nilai:=-q_search(3-giliran,-t_beta,-t_alpha,data,1,depth+1,cext,neval,-nodetype)
        else
          nilai:=-q_search(3-giliran,-t_beta,-t_alpha,data,1,depth+1,cext,neval,node_Cut)
      end
      else
      begin
        if legalmove=1 then
          nilai:=-mainsearch(newlevel,3-giliran,-t_beta,-t_alpha,data,false,depth+1,cext,nmtree,areduce,-nodetype)
        else
          nilai:=-mainsearch(newlevel,3-giliran,-t_beta,-t_alpha,data,false,depth+1,cext,nmtree,areduce,node_cut);
      end;
search2:
      begin

        if  (nilai>t_alpha) {and (level>1)} and (nilai<beta) and (legalmove>1) then
        begin
         {$IFDEF stat}
          inc(rs);
          {$ENDIF}
{          if futflag then
            inc(futfail);}
          if areduce<>0 then
            inc(newlevel,areduce);
          if newlevel<FRACTIONAL_PLY then
          begin
            nilai:=-q_search(3-giliran,-beta,-t_alpha,data,1,depth+1,cext,noval,node_PV);
          end
          else
            nilai:=-mainsearch(newlevel,3-giliran,-beta,-t_alpha,data,false,depth+1,cext,nmtree,areduce,node_PV);
        end else
        if nilai>=beta then
        begin
          if areduce<>0 then
          begin
            inc(newlevel,areduce);
            areduce:=0;
            if newlevel<FRACTIONAL_PLY then
            begin
              nilai:=-q_search(3-giliran,-nilai,-t_alpha,data,1,depth+1,cext,noval,node_PV);
            end
            else
              nilai:=-mainsearch(newlevel,3-giliran,-nilai,-t_alpha,data,false,depth+1,cext,nmtree,areduce,NODe_PV);
          end;
        end;
search3:
        if nilai>g then
        begin
          g:=nilai;
          isScoreDraw:=ScoreDraw;
          if nilai>t_alpha then
          begin
            tmoves:=moves;

            t_alpha:=nilai;
            if g>=beta then
            begin
{$IFDEF USEFUTIL}
              if data.cp=0 then
                inc(hist_succ[get_moving_piece,get_dest(sto)],rlevel*rlevel);
{$ENDIF}
              if ((data.cp=0)) and not incheck then
                addhistory(rlevel,depth,moves,giliran,get_moving_piece);
              {$IFDEF stat}
             inc(totalco);
             if legalmove=1 then inc(cofirst);
             {$ENDIF}
             delrephash(data.hashkey,2-giliran);
             data:=temp;
             break;
           end;
          end;
        end;
        t_beta:=t_alpha+1;
      end; //tipe=_NODEMAX
      delrephash(data.hashkey,2-giliran);
      data:=temp;
      {if nodetype=NODE_CUT then
        nodeType:=NODE_ALL;}
    until 1=2;
    if (timepassed>=timelimit) or stop_process then
    begin
      exit;
    end;
    if legalmove=0 then
    begin
      tmoves:=_NO_MOVE;
      result:=contemp_draw;
      g:=result;
    end else
    begin
      result:=g;
      if tmoves=_NO_MOVE then
        tmoves:=hmoves;
    end;
    if not isScoreDraw then
    begin
      IF g<=alpha THEN
      begin
         {$IFDEF stat}
         inc(fl);
         {$ENDIF}
         addtable(h,giliran,level,tmoves,g,FAIL_LOW);

      end ELSE if g>=beta then
      begin
          addtable(h,giliran,level,tmoves,g,FAIL_HIGH);
          {$IFDEF stat}
          inc(fh);
         {$ENDIF}
      end
      else
      begin
          addtable(h,giliran,level,tmoves,g,PV_NODE);
          {$IFDEF stat}
          inc(pvn);
         {$ENDIF}
         sto:=tmoves shr 7 and 127;
         if  (tmoves<>_NO_MOVE) and (sto<>_EN_PASSANT)
         and ((sto>63) or (data.papan[sto]=0))
         then
         begin
          if giliran=_SISIPUTIH then
          begin
//            assert(data.papan[tmoves and 127]>0);
            inc(historyw[data.papan[tmoves and 127],tmoves shr 7 and 127],(rlevel)*rlevel )
          end
          else
          begin
//            assert(data.papan[tmoves and 127]<0);
            inc(historyb[data.papan[tmoves and 127],tmoves shr 7 and 127],(rlevel)*rlevel );
          end;
         end;

      end;
    end;
end;


function Q_search;
var
temp:tdata;
ml:tmovelist;
tstat:byte;
incheck,nganu:boolean;
V,tempnm,sc,t_alpha,t_beta,g,a,jml,nilai,e,sto,sfrom:integer;
abcd:Boolean;

function cap_is_danger:Boolean;
begin
  if giliran=_SISIPUTIH then
  begin
    if data.papan[sto]=_mentriHITAM then
    begin
      result:=true;
      exit;
    end;
    if (ml[a].score>0) and (kingmask[sto] and data.kingblack<>0)
    and (data.queenwhite<>0)
    then
    begin
      result:=true;
      exit;
    end;
    if (data.papan[ml[a].moves and 127]=1)
    and (sto>=a7) then
    begin
      result:=true;
      exit;
    end;
    if (data.papan[sto]=-1)
    and (sto<=h2) then
    begin
      result:=true;
      exit;
    end;

  end else
  begin
    if data.papan[sto]=_mentriPUTIH then
    begin
      result:=true;
      exit;
    end;
    if (ml[a].score>0) and (kingmask[sto] and data.kingwhite<>0)
    and (data.queenblack<>0)
    then
    begin
      result:=true;
      exit;
    end;
    if (data.papan[ml[a].moves and 127]=-1)
    and (sto<=h2) then
    begin
      result:=true;
      exit;
    end;
    if (data.papan[sto]=1)
    and (sto>=a7) then
    begin
      result:=true;
      exit;
    end;

  end;
  result:=false;
end;

begin
  if giliran=_SISIPUTIH then
     incheck:=white_checked(data)
  else
     incheck:=black_checked(data);

  if not incheck then
  begin
    if neval=noval then
      e:=eval(data,giliran,alpha,beta)
    else
      e:=neval;

    begin
      if e>=beta then
      begin
        result:=e;
        exit;
      end;

      if (e>alpha) then
        alpha:=e;
    end;
  end;

  if giliran=_SISIPUTIH then
  begin
      if incheck then
      begin
        white_evasion(data,ml,jml,sc);
        case jml of
         1: inc(ply);
         0: begin
              result:=-_NILAI_RAJA+depth;
              exit;
            end;
         end;
      end
      else
      begin
        if ply>0 then
          q_white_movgen(data,ml,jml)
        else
          white_movgen_caps2(data,ml,jml);
        if jml=0 then
        begin
//          result:=e;exit;
        end;
      end;
  end else
  begin
      if incheck then
      begin
        black_evasion(data,ml,jml,sc);
        case jml of
         1: inc(ply);
         0: begin
              result:=-_NILAI_RAJA+depth;
              exit;
            end;
         end;
      end
      else
      begin
        if ply>0 then
          q_black_movgen(data,ml,jml)
        else
          black_movgen_caps2(data,ml,jml);
        if jml=0 then
        begin
//          result:=e;exit;
        end;
      end;
  end;
  temp:=data;

  nganu:=false;
  for a:=1 to jml do
    if ml[a].score>0 then
    begin
      nganu:=true;break;
    end;
  if not nganu then
  begin
    if (ply>0) and (cext>0) and (e<beta)
    then
    begin
      nilai:=materialvalue(giliran,data.materialscore);
      if (e-nilai>=90)
      //if random(20)=1
      //if ((e-nilai>=90) ) or ((nilai-e>=90) )
      then
      begin
        if e-nilai>=200 then
          result:=mainsearch(16,giliran,alpha,beta,data,false,depth,0,false,0,nodeType)
        else
          result:=mainsearch(8,giliran,alpha,beta,data,false,depth,0,false,0,nodeType);
        exit;
      end else
      if jml=0 then
      begin
        result:=e;exit;
      end;
    end else
    if jml=0 then
    begin
      result:=e;exit;
    end;
  end;


//  if (ply=2) and not incheck then ply:=0;

  if not incheck then
    g:=e
  else
  begin
     g:=-_INFINITY-100;
  end;
  t_alpha:=alpha;
  t_beta:=beta;

  for a:=1 to jml do
  begin
    if a<>jml then
    nextbest_max(ml,a,jml);
    abcd:=false;
    if not incheck then
    begin
      if ml[a].score<0 then
      begin
//        data:=temp;
        break;
      end;

      sto:=(ml[a].moves shr 7) and 127;
      if (alpha=beta-1) and (sto<=63) and (data.papan[sto]<>0)
      then
      begin
        v:=e+nilai_piece[data.papan[sto]]+70;
        if (V<t_alpha) and not menskak2(temp,giliran,ml[a].moves)
        and not pawn_7_rank(giliran,data)
        and (((ml[a].moves shr 14) and 7)=0) then
        begin
          if not cap_is_danger then
          begin
            if V>g then
              g:=V;
            continue

          end;
        end;
      end;

      sfrom:=ml[a].moves and 127;

      if giliran=_SISIPUTIH then
      begin
          if (data.papan[sfrom]=_RAJAPUTIH) and white_attacked(data,sto) then
            continue else
          if white_open_check(data,ml[a].moves) then continue;
      end else
      begin
          if (data.papan[sfrom]=_RAJAHITAM) and black_attacked(data,sto) then
            continue
          else
          if black_open_check(data,ml[a].moves) then continue;
      end;
    end;  //if not incheck


    if giliran=_SISIPUTIH then
      makewhitemove(ml[a].moves,data)
    else
      makeblackmove(ml[a].moves,data);

    if sto=_EN_PASSANT then
    begin
      if ((giliran=_SISIPUTIH) and white_checked(data))
      or ((giliran=_SISIHITAM) and black_checked(data)) then
      begin
        data:=temp;
//        dec(total_node);
        continue;
      end;
    end;


    nilai:=-q_search(3-giliran,-t_beta,-t_alpha,data,ply-1,depth+1,cext,noval,-nodetype);
    begin
      if nilai>=beta then
      begin
        data:=temp;
        result:=nilai;exit;
      end;
      if nilai>g then
      begin
        g:=nilai;
        if nilai>t_alpha then
        begin
          t_alpha:=nilai;
        end;
      end;
    end;
    data:=temp;
  end;     //for a;
  result:=g

end;

end.
