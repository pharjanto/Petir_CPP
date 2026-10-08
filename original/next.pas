unit next;

interface
uses header;

procedure nextbest_max(var a:tmovelist;cur,tot:integer);
function nextevasion(giliran:integer;var data:tdata;var moves,fase,hmoves,curmove,jml,skakcount:integer;var ml:tmovelist):boolean;
function nextmove(giliran,depth:integer;var data:tdata;var moves,fase:integer;hmoves:integer;var curmovecap,curmovenoncap,jmlcap,jmlnoncap:integer;var ml,mlcap:tmovelist;var goodcap:integer):boolean;
function nextroot(giliran:integer;var moves,fase,hmoves,curmove,jml:integer;ml:tmovelist):boolean;
function nextq(giliran:integer;var data:tdata;var moves,fase,curmove,jml,score:integer;ply:integer;var ml:tmovelist):boolean;
implementation
uses movgen,valid,hashing_header,makemove,usee,tools;
var t1:integer=0;

function nextq(giliran:integer;var data:tdata;var moves,fase,curmove,jml,score:integer;ply:integer;var ml:tmovelist):boolean;
begin
  result:=false;
  if fase=0 then
  begin
    if giliran=_SISIPUTIH then
    begin
      if ply>0 then
        q_white_movgen(data,ml,jml)
      else
        white_movgen_caps(data,ml,jml);
    end else
    begin
      if ply>0 then
        q_black_movgen(data,ml,jml)
      else
        black_movgen_caps(data,ml,jml);
    end;
    inc(fase);
    if jml=0 then
    begin
      result:=true;exit;
    end;
  end;
  if fase=1 then
  begin

    if curmove>jml then
    begin
      moves:=_NO_MOVE;exit;
    end;
    nextbest_max(ml,curmove,jml);
    moves:=ml[curmove].moves;
    score:=ml[curmove].score;
    inc(curmove);
    exit;
  end;
end;

procedure nextbest_max(var a:tmovelist;cur,tot:integer);
//partial ordering
var i,max,b:integer;
temp:tmoverecord;
begin
  max:=a[cur].score;
  b:=cur;
  for i:=cur+1 to tot do
  begin
    if a[i].score>max then
    begin
      b:=i;
      max:=a[i].score;
    end;
  end;
  if cur<>b then
  begin
    temp:=a[cur];
    a[cur]:=a[b];
    a[b]:=temp;
  end;
end;

function nextroot(giliran:integer;var moves,fase,hmoves,curmove,jml:integer;ml:tmovelist):boolean;
begin
  if (fase=_HASHMOVE) then
  begin
    inc(fase);
    if isvalid(data,hmoves,giliran) then
    begin
      moves:=hmoves;
      exit;
    end;
  end;
  if fase=_HASHMOVE+1 then
  begin
    inc(curmove);
    if curmove>jml then
    begin
      moves:=_NO_MOVE;exit;
    end;
    nextbest_max(ml,curmove,jml);
    if (ml[curmove].moves=hmoves)  then
    begin
      inc(curmove);
      if (curmove>jml)  then begin
        moves:=_NO_MOVE;exit;
      end
      else
      begin
        nextbest_max(ml,curmove,jml);
      end;
    end;
    moves:=ml[curmove].moves;exit;

  end;

end;

function nextevasion;
var a:integer;
label habis;
begin
 result:=false;
 if (fase=_HASHMOVE) then
 begin
    if curmove=0 then
    begin
      if giliran=_SISIPUTIH then white_evasion(data,ml,jml,skakcount)
      else black_evasion(data,ml,jml,skakcount);
      if jml=0 then
      begin
        result:=true;exit;
      end;
      for a:=1 to jml do
        if ml[a].moves=hmoves then
        begin
          inc(ml[a].score,87000000);
          break;
        end;
    end;

    inc(curmove);
    if curmove>jml then
    begin
      moves:=_NO_MOVE;exit;
    end else
    begin
      nextbest_max(ml,curmove,jml);
      moves:=ml[curmove].moves;
      exit;
    end;

 end;
 habis:
 if fase=_HASHMOVE+2 then
 begin
   moves:=_NO_MOVE;exit;
 end;
end;

function nextmove;
var a:integer;
killer0moves,killer1moves,nilai:integer;
killer0moves2,killer1moves2:integer;
found,sudahada:boolean;
//t2:tdata;

label other,fase3,fase4,badcap,killer11,killer21,killer31,killer41;
begin
  //fungsi nextmove digunakan untuk menentukan langkah yang akan diperiksa selanjutnya
  //dan juga untuk melakukan move ordering
  //skema move ordering :
  //1. Prioritas pertama adalah langkah dari hash move
  //2. langkah goodcaps yaitu langkah memakan yang nilai SEE >=0 atau langkah pion promosi
  //3. langkah killermove, ada 4 killer yaitu 2 dari ply saat ini dan dua lagi dari ply-2
  //4. sisa langkah lainnnya yg disortir berdasarkan nilai history heuristic
  //5. langkah badcap yaitu langkah memakan yg nilai see<0
  result:=false;
//  mbadcap:=false;
  if (fase=_HASHMOVE) then
  begin
    inc(fase);
    //coba langkah dari hash table
    //cek dulu apakah langkah dari hash table valid atau tidak
    //sebagai tambahan pengaman extra
    //jika terjadi error pada key hash table (2 posisi berbeda mempunyai hash key yg sama)
//    if isvalid(data,hmoves,giliran) then
    if hmoves<>0 then
    begin
      moves:=hmoves;
      exit;
    end;
  end;
  if fase=_GOODCAPS then
  begin
  //coba langkah good capture yaitu langkah yg nilai SEE>=0
  //nilai SEE dan history heuristic untuk tiap langkah sudah dihitung pada saat movgen
    if curmovecap=0 then
    begin

      // movgen untuk langkah memakan dan untuk non langkah memakan dibedakan
      // tujuannya untuk meningkatkan kecepatan, karena sebagian besar cutoff dihasilkan
      // oleh langkah memakan, shg jika langkah memakan menghasilkan cutoff maka tidak perlu
      //  lagi menggenerate langkah non memakan yang jumlahnya jauh lebih banyak
      if giliran=_SISIPUTIH then white_movgen_caps(data,mlcap,jmlcap)
      else black_movgen_caps(data,mlcap,jmlcap);
    end;
    inc(curmovecap);
    if curmovecap>jmlcap then
    begin
      inc(fase);
      curmovenoncap:=0;
    end else
    begin
      //sortir langkah memakan menurut nilai SEE atau LVV/MVAnya
      //sortir ini dilakukan secara bertingkat, setiap kali sortir hanya dicari 1 langkah terbaik
      //yang ditempatkan di urutan depan. Sehingga jika suatu langkah menghasilkan
      //cutoff maka tidak perlu mensortir keseluruhan isi array
      nextbest_max(mlcap,curmovecap,jmlcap);
      //kalau langkah yang akan diambil sama dengan langkah dari hash table
      //maka pindah ke langkah berikutnya
      if (mlcap[curmovecap].moves=hmoves)  then
      begin
        inc(curmovecap);
        if (curmovecap>jmlcap)  then begin
          inc(fase);
          curmovenoncap:=0;
          goto other;
        end
        else
        begin
          nextbest_max(mlcap,curmovecap,jmlcap);
        end;
      end;
      if mlcap[curmovecap].score<0 then
      begin
badcap:
          inc(fase);
          curmovenoncap:=0;
          goto other;
      end;

      moves:=mlcap[curmovecap].moves;
      if mlcap[curmovecap].score>goodcap then
        goodcap:=mlcap[curmovecap].score;
      exit;
    end;
  end;

other:
  //coba langkah killer move
  //keuntungan tambahan dari killer move adalah jika salah satu dari 4 killer menghasilkan
  //cutoff, maka tidak perlu memanggil movgen untuk noncaps sehingga dpt menghemat waktu
  if fase=_KILLERMOVES1 then
  begin
    inc(fase);
    //cek apapakah langkah killer valid?
    if (killer0[depth]<>hmoves) and
    isvalid(data,killer0[depth],giliran)
//    and not capture(data,killer0[depth])
    then
    begin
      for a:=1 to curmovecap-1 do
        if killer0[depth]=mlcap[a].moves then
          goto killer11;
      moves:=killer0[depth];
      exit;
    end;
  end;
killer11:
  if fase=_KILLERMOVES2 then
  begin
    inc(fase);
    if (killer1[depth]<>hmoves) and
    isvalid(data,killer1[depth],giliran)
//    and not capture(data,killer1[depth])
     then
    begin
      for a:=1 to curmovecap-1 do
        if killer1[depth]=mlcap[a].moves then
          goto killer21;

      moves:=killer1[depth];
      exit;
    end;
  end;
killer21:
  if fase=_KILLERMOVES3 then
  begin
    inc(fase);
    if (killer0[depth-2]<>hmoves) and
    isvalid(data,killer0[depth-2],giliran)
    and (killer0[depth-2]<>killer0[depth])
    and (killer0[depth-2]<>killer1[depth])
//    and not capture(data,killer0[depth-2])
    then
    begin
      for a:=1 to curmovecap-1 do
        if killer0[depth-2]=mlcap[a].moves then
          goto killer31;

      moves:=killer0[depth-2];
      exit;
    end;
  end;
killer31:
  if fase=_KILLERMOVES4 then
  begin
    inc(fase);
    if (killer1[depth-2]<>hmoves) and
    isvalid(data,killer1[depth-2],giliran)
    and (killer1[depth-2]<>killer0[depth])
    and (killer1[depth-2]<>killer1[depth])
//    and not capture(data,killer1[depth-2])
    then
    begin
      for a:=1 to curmovecap-1 do
        if killer1[depth-2]=mlcap[a].moves then
          goto killer41;

      moves:=killer1[depth-2];
      exit;
    end;
  end;

killer41:
  if fase=_OTHERMOVES then
  begin
  //masih belum terjadi cutoff, maka coba langkah non capture
    if curmovenoncap=0 then
    begin
      if giliran=_SISIPUTIH then white_movgen_noncaps(data,ml,jmlnoncap,depth)
      else black_movgen_noncaps(data,ml,jmlnoncap);

    end;
    inc(curmovenoncap);
    if curmovenoncap>jmlnoncap then
    begin
       inc(fase);
    end else
    begin
      // hanya disortir 20 langkah terbaik, sisanya gak disortir biar lebih banter
      if curmovenoncap<=20 then
        nextbest_max(ml,curmovenoncap,jmlnoncap);
      //cari sampai langkah yang akan diperiksa bukan langkah hash move atau langkah
      //killer move
      while (ml[curmovenoncap].moves =hmoves)
      or (ml[curmovenoncap].moves =killer0[depth])
      or (ml[curmovenoncap].moves =killer1[depth])
      or (ml[curmovenoncap].moves =killer0[depth-2])
      or (ml[curmovenoncap].moves =killer1[depth-2])
      do
      begin
        inc(curmovenoncap);
        if curmovenoncap>jmlnoncap then begin
          inc(fase);
          goto fase3;
        end
        else
          if curmovenoncap<=20 then
             nextbest_max(ml,curmovenoncap,jmlnoncap);
      end;
      moves:=ml[curmovenoncap].moves;
      exit;
    end;
  end;
fase3:
  //langkah badcap yaitu langkah yang nilai SEE<0
  //langkah badcap jika menghasilkan cutoff juga disimpan dalam killer move
  if fase=_BADCAPS then
  begin
    if curmovecap<=jmlcap then
    begin
      nextbest_max(mlcap,curmovecap,jmlcap);
      while (mlcap[curmovecap].moves=hmoves)
      or (mlcap[curmovecap].moves=killer0[depth])
      or (mlcap[curmovecap].moves=killer1[depth])
      or (mlcap[curmovecap].moves=killer0[depth-2])
      or (mlcap[curmovecap].moves=killer1[depth-2])
      do
      begin
        inc(curmovecap);
        if curmovecap>jmlcap then
        begin
          inc(fase);
          goto fase4;
        end;
        nextbest_max(mlcap,curmovecap,jmlcap);
      end;
      moves:=mlcap[curmovecap].moves;
      inc(curmovecap);
//      mbadcap:=true;
      exit;

    end else
      inc(fase);
  end;
fase4:
  if fase=_FINISH then
  begin
    moves:=_NO_MOVE;
  end;
end;



end.
