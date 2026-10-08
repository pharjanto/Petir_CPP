unit root;
{$define testing}
{$DEFINE USEFUTIL}

interface
uses header;

function searchroot(level,giliran,tipe,alpha,beta:integer;data:tdata;nullmove:boolean;depth,cext:integer):integer;

var bmoves:integer;
ntimelimit,rtl:integer;
newpv:boolean;

implementation
uses makemove,hashing,next,tools,search,pv,windows,math,notation,repetition,movgen;
function searchroot;
var g,fase,t_alpha,t_beta,nilai,tempi:integer;
anu,ext,incheck,first,finish,pp,draw,mbadcap:boolean;
temp:tdata;
cp:shortint;
goodcap,tnilai:integer;
ml,mlcap:tmovelist;spv:string;
lmoves,legalmove,nl,moves,hmoves,curmovecap,curmovenoncap,jmlcap,jmlnoncap,newlevel,nlev:integer;
h2,h:int64;
sc,cnode:integer;
tcext,rlevel,sto:integer;
eext:integer;
stat:byte;
label search1,search2;
begin

    rlevel:=level div FRACTIONAL_PLY;
    fillchar(curr_move,sizeof(curr_move),0);
    cnode:=total_node;
    hmoves:=_NO_MOVE;
    if searchhash(data.hashkey,alpha,beta,level,giliran,nilai,hmoves,mbadcap) then
    begin
      inc(hashco);mmoves:=hmoves;
      newpv:=false;
{      if not use_mtdf then
      begin
        spv:=getpv(data,giliran,level div FRACTIONAL_PLY);
        printpv(barisitr,spv,level div FRACTIONAL_PLY,floor((gettickcount-startclock)/ 1000),nilai);
      end;}
      result:=nilai;exit;
    end;
    h:=data.hashkey;

    inc(total_node);
    finish:=false;
    fase:=0;
    temp:=data;
    curmovecap:=0;
    t_alpha:=alpha;t_beta:=beta;
    if tipe=_NODEMAX then
     g:=-_INFINITY-100
    else g:=_INFINITY+100;
    first:=true;
    legalmove:=0;
    nlev:=level-FRACTIONAL_PLY;
    eext:=0;

    if giliran=_SISIPUTIH then
    begin
      if white_checked(data) then
      begin
        incheck:=true;
      end else
      begin
        incheck:=false;
      end;
    end else
    begin
      if black_checked(data) then
      begin
        incheck:=true;
      end else
      begin
        incheck:=false;
      end;
    end;
    tcext:=cext;
    {$IFDEF USEFUTIL}
    empty_HistoryTable;
    {$ENDIF}

    repeat
      stack[depth].Ext:=false;
      fillchar(stack,sizeof(stack),0);
      cext:=tcext;
      ext:=false;draw:=false;
      newlevel:=nlev;
      if not incheck then
        nextmove(giliran,depth,data,moves,fase,hmoves,curmovecap,curmovenoncap,jmlcap,jmlnoncap,ml,mlcap,goodcap)
      else
      begin
        if nextevasion(giliran,data,moves,fase,hmoves,curmovecap,jmlcap,sc,mlcap) then
        begin
          result:=-_NILAI_RAJA-rlevel;
          exit;
        end;
        if (cext>0) then
        begin
          stack[depth].Ext:=true;
          if (jmlcap=1) then
          begin
            inc(newlevel,SINGLE_REPLY_EXTENSION);
            dec(cext,SINGLE_REPLY_EXTENSION);
          end else
          if (jmlcap=2) then
          begin
            inc(newlevel,TWO_REPLY_EXTENSION);
            dec(cext,TWO_REPLY_EXTENSION);
          end;
          if sc=2 then
          begin
            inc(newlevel,DOUBLE_SKAK_EXTENSION);
            dec(cext,DOUBLE_SKAK_EXTENSION);
          end;
          inc(newlevel,SKAK_EXTENSION);
          dec(cext,SKAK_EXTENSION);
          ext:=true;
        end;

      end;
      if moves=_NO_MOVE then
      begin
         break;
      end;


      sto:=moves shr 7 and 127;

      if sto <=63 then
          cp:=data.papan[sto]
      else if sto=_EN_PASSANT then cp:=1
      else cp:=0;
      if (cp=0) and ((moves shr 14) and 7<>0) then cp:=1;

      if giliran=_SISIPUTIH then
      begin
        makewhitemove(moves,data);
        if white_checked(data) then
        begin
          data:=temp;
          continue;
        end;
      end else
      begin
        makeblackmove(moves,data);
        if black_checked(data) then
        begin
          data:=temp;
          continue;
        end;
      end;
      inc(legalmove);
      stack[depth].fase:=fase;
      stack[depth].legalmove:=legalmove;
      stack[depth].mte:=false;
      h2:=data.hashkey;
      if addrephash(data.hashkey,2-giliran)=2 then
      begin
        nilai:=0;
        draw:=true;
        goto search2;
      end;

      curr_move[depth]:=moves;

      if not ext and (sto<=63) and (abs(data.papan[sto])=_PIONPUTIH) and IsExtension(data,newlevel,giliran,moves,cext) then
      begin
        stack[depth].Ext:=true;
        goto search1;
      end;
search1:
      if legalmove=1 then
        nilai:=-mainsearch(newlevel,3-giliran,-t_beta,-t_alpha,data,nullmove,depth+1,cext,false,0,NODe_PV)
      else
        nilai:=-mainsearch(newlevel,3-giliran,-t_beta,-t_alpha,data,nullmove,depth+1,cext,false,0,NODe_CUT);
      tnilai:=nilai;
search2:
      if ((timepassed>=timelimit)) or stop_process then
      begin
        delrephash(h2,2-giliran);
        data:=temp;
        exit;
      end;
      lmoves:=mmoves;
      if tipe=_NODEMAX then
      begin
        if not use_mtdf and (nilai>t_alpha) {and (level>1)} and (nilai<beta) and not first and not draw then
        begin
          mmoves:=moves;
            {$ifdef testing}
            if runningtest then
            begin
              for sc:=1 to 5 do if moves=testanswer[sc] then
                testsolved:=true;
            end;
            {$endif}

          nilai:=-mainsearch(newlevel,3-giliran,-beta,-t_alpha,data,nullmove,depth+1,cext,false,0,NODe_PV);
          if ((timepassed>=timelimit)) or stop_process then
          begin
            delrephash(h2,2-giliran);
            data:=temp;
            exit;
          end;
          if nilai<tnilai then
          begin
            mmoves:=lmoves;
//            nilai:=tnilai;
          end;

        end;
        if nilai>g then
        begin
          g:=nilai;
          if nilai>t_alpha then
          begin
            if not use_mtdf then
            begin
              spv:=getpv(data,3-giliran,rlevel+3);
              spv:=movetonotation(moves,temp.ep,giliran)+' '+spv;
              printpv(barisitr,spv,rlevel,floor((gettickcount-startclock)/ 1000),nilai);
            end;
            {$ifdef testing}
            if runningtest then
            begin
              for sc:=1 to 5 do if moves=testanswer[sc] then
                testsolved:=true;
            end;
            {$endif}
            if lmoves<>moves then
            begin
              if ((modetime=MOVES_MINUTES) or (modetime=MINUTES_GAME_INCREMENT)) and (eext<=6)
              then
              begin
                inc(timelimit,rtl div 10);
                inc(eext);
              end;
            end;
            mmoves:=moves;
            if mmoves<>bmoves then
              bmoves:=_NO_MOVE;
            t_alpha:=nilai;
            if g>=beta then
            begin
              if cp=0 then
              begin
                if giliran=_SISIPUTIH then
                  inc(historyw[data.papan[moves shr 7 and 127],moves shr 7 and 127],rlevel*rlevel)
                else
                  inc(historyb[data.papan[moves shr 7 and 127],moves shr 7 and 127],rlevel*rlevel);
                if (killer0[depth]<>moves) then
                begin
                  killer1[depth]:=killer0[depth];
                  killer0[depth]:=moves;
               end;
             end;
             inc(totalco);
             if first then inc(cofirst);
             delrephash(h2,2-giliran);
             data:=temp;
             result:=g;break;
           end;
          end;
        end;
        if not use_mtdf then t_beta:=t_alpha+1;
      end; //tipe=_NODEMAX
      first:=false;
      delrephash(h2,2-giliran);
      data:=temp;
    until finish;

    if legalmove=0 then
    begin
        result:=contemp_draw;
        g:=result;
        stalemate:=true;
    end;
    result:=g;
    if mmoves=_NO_MOVE then
      mmoves:=hmoves;
    if ((timepassed>=timelimit)) or stop_process then
      exit;
    IF g<=alpha THEN
    begin
      addtable(h,giliran,level,mmoves,g,FAIL_LOW);
    end ELSE if g>=beta then
    begin
        addtable(h,giliran,level,mmoves,g,FAIL_HIGH)
    end
    else
      addtable(h,giliran,level,mmoves,g,PV_NODE);
end;



end.
