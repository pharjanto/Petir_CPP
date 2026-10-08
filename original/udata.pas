unit udata;


interface
uses header;


  function map(y,x:byte):byte;
  Procedure initstate(var data:tdata);

implementation

uses bitboard_mask,hashing_header;


function map(y,x:byte):byte;
begin
	map:=(y*8+x);
end;

Procedure initstate(var data:tdata);
var a,b:byte;
begin
   fillchar(data,sizeof(data),10);
   for a:=0 to 7 do
   for b:=0 to 7 do
        data.papan[map(b,a)]:=0;

   with data do begin
//        jumlah_langkah:=0;
        flagrokade:=15;
//        bishop_oppc:=false;
        move50count:=0;
        wc:=NOROKADE;bc:=NOROKADE;
        pawnending:=false;
        ngajahputih:=2;ngajahhitam:=2;

        for a:=0 to 7 do
        begin
                papan[map(1,a)]:=_PIONPUTIH;
                papan[map(6,a)]:=_PIONHITAM;
        end;

        papan[map(0,0)]:=_BENTENGPUTIH;
        papan[map(0,7)]:=_BENTENGPUTIH;
        papan[map(7,0)]:=_BENTENGHITAM;
        papan[map(7,7)]:=_BENTENGHITAM;
        papan[map(0,1)]:=_KUDAPUTIH;
        papan[map(0,6)]:=_KUDAPUTIH;
        papan[map(7,1)]:=_KUDAHITAM;
        papan[map(7,6)]:=_KUDAHITAM;
        papan[map(0,2)]:=_GAJAHPUTIH;
        papan[map(0,5)]:=_GAJAHPUTIH;
        papan[map(7,2)]:=_GAJAHHITAM;
        papan[map(7,5)]:=_GAJAHHITAM;
        papan[map(0,3)]:=_MENTRIPUTIH;
        papan[map(7,3)]:=_MENTRIHITAM;
        papan[map(0,4)]:=_RAJAPUTIH;
        papan[map(7,4)]:=_RAJAHITAM;
        {kingputihpos:=map(0,4);
        kinghitampos:=map(7,4);}
        ep:=_no_ep;
//        jumlahbijicaturtambahan:=0;




//        data.hashkey:=0;
//        data.jum_materi_putih:=16;
//        data.jum_materi_hitam:=16;

//-------menginisialisasi bitboard----------
//inisialisasi whitepieces
        whitepieces:=0;blackpieces:=0;allpiecesr90:=0;allpiecesa8h1:=0;allpiecesh8a1:=0;
        kingblack:=0;kingwhite:=0;pawnblack:=0;pawnwhite:=0;
        knightblack:=0;knightwhite:=0;bishopblack:=0;bishopwhite:=0;
        rookblack:=0;rookwhite:=0;queenblack:=0;queenwhite:=0;
{        for a:=1 to 6 do
        begin
          whitepiece2[a]:=0;
          blackpiece2[a]:=0;
        end;}
//inisialisasi semua buah catur putih
        whitepieces:=whitepieces or bit2nmask[a1];
        whitepieces:=whitepieces or bit2nmask[b1];
        whitepieces:=whitepieces or bit2nmask[c1];
        whitepieces:=whitepieces or bit2nmask[d1];
        whitepieces:=whitepieces or bit2nmask[e1];
        whitepieces:=whitepieces or bit2nmask[f1];
        whitepieces:=whitepieces or bit2nmask[g1];
        whitepieces:=whitepieces or bit2nmask[h1];
        whitepieces:=whitepieces or bit2nmask[a2];
        whitepieces:=whitepieces or bit2nmask[b2];
        whitepieces:=whitepieces or bit2nmask[c2];
        whitepieces:=whitepieces or bit2nmask[d2];
        whitepieces:=whitepieces or bit2nmask[e2];
        whitepieces:=whitepieces or bit2nmask[f2];
        whitepieces:=whitepieces or bit2nmask[g2];
        whitepieces:=whitepieces or bit2nmask[h2];
//inisialisasi pion putih
{        whitepiece2[pawn]:=whitepiece2[a] or bit2nmask2[38];
        whitepiece2[pawn]:=whitepiece2[a] or bit2nmask2[39];
        whitepiece2[pawn]:=whitepiece2[a] or bit2nmask2[40];
        whitepiece2[pawn]:=whitepiece2[a] or bit2nmask2[41];
        whitepiece2[pawn]:=whitepiece2[a] or bit2nmask2[42];
        whitepiece2[pawn]:=whitepiece2[a] or bit2nmask2[43];
        whitepiece2[pawn]:=whitepiece2[a] or bit2nmask2[44];
        whitepiece2[pawn]:=whitepiece2[a] or bit2nmask2[45];

        whitepiece2[knight]:=whitepiece2[a] or bit2nmask2[38];}
//inisialisasi blackpieces
        blackpieces:=blackpieces or bit2nmask[a7];
        blackpieces:=blackpieces or bit2nmask[b7];
        blackpieces:=blackpieces or bit2nmask[c7];
        blackpieces:=blackpieces or bit2nmask[d7];
        blackpieces:=blackpieces or bit2nmask[e7];
        blackpieces:=blackpieces or bit2nmask[f7];
        blackpieces:=blackpieces or bit2nmask[g7];
        blackpieces:=blackpieces or bit2nmask[h7];
        blackpieces:=blackpieces or bit2nmask[a8];
        blackpieces:=blackpieces or bit2nmask[b8];
        blackpieces:=blackpieces or bit2nmask[c8];
        blackpieces:=blackpieces or bit2nmask[d8];
        blackpieces:=blackpieces or bit2nmask[e8];
        blackpieces:=blackpieces or bit2nmask[f8];
        blackpieces:=blackpieces or bit2nmask[g8];
        blackpieces:=blackpieces or bit2nmask[h8];
//inisialisasi allpiece rotated 90
        allpiecesr90:=allpiecesr90 or bit2nmask90[a1];
        allpiecesr90:=allpiecesr90 or bit2nmask90[b1];
        allpiecesr90:=allpiecesr90 or bit2nmask90[c1];
        allpiecesr90:=allpiecesr90 or bit2nmask90[d1];
        allpiecesr90:=allpiecesr90 or bit2nmask90[e1];
        allpiecesr90:=allpiecesr90 or bit2nmask90[f1];
        allpiecesr90:=allpiecesr90 or bit2nmask90[g1];
        allpiecesr90:=allpiecesr90 or bit2nmask90[h1];
        allpiecesr90:=allpiecesr90 or bit2nmask90[a2];
        allpiecesr90:=allpiecesr90 or bit2nmask90[b2];
        allpiecesr90:=allpiecesr90 or bit2nmask90[c2];
        allpiecesr90:=allpiecesr90 or bit2nmask90[d2];
        allpiecesr90:=allpiecesr90 or bit2nmask90[e2];
        allpiecesr90:=allpiecesr90 or bit2nmask90[f2];
        allpiecesr90:=allpiecesr90 or bit2nmask90[g2];
        allpiecesr90:=allpiecesr90 or bit2nmask90[h2];

        allpiecesr90:=allpiecesr90 or bit2nmask90[a7];
        allpiecesr90:=allpiecesr90 or bit2nmask90[b7];
        allpiecesr90:=allpiecesr90 or bit2nmask90[c7];
        allpiecesr90:=allpiecesr90 or bit2nmask90[d7];
        allpiecesr90:=allpiecesr90 or bit2nmask90[e7];
        allpiecesr90:=allpiecesr90 or bit2nmask90[f7];
        allpiecesr90:=allpiecesr90 or bit2nmask90[g7];
        allpiecesr90:=allpiecesr90 or bit2nmask90[h7];
        allpiecesr90:=allpiecesr90 or bit2nmask90[a8];
        allpiecesr90:=allpiecesr90 or bit2nmask90[b8];
        allpiecesr90:=allpiecesr90 or bit2nmask90[c8];
        allpiecesr90:=allpiecesr90 or bit2nmask90[d8];
        allpiecesr90:=allpiecesr90 or bit2nmask90[e8];
        allpiecesr90:=allpiecesr90 or bit2nmask90[f8];
        allpiecesr90:=allpiecesr90 or bit2nmask90[g8];
        allpiecesr90:=allpiecesr90 or bit2nmask90[h8];

        //inisialisasi all piece rotated h8a1
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[a1];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[b1];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[c1];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[d1];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[e1];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[f1];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[g1];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[h1];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[a2];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[b2];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[c2];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[d2];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[e2];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[f2];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[g2];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[h2];

        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[a7];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[b7];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[c7];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[d7];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[e7];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[f7];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[g7];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[h7];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[a8];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[b8];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[c8];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[d8];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[e8];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[f8];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[g8];
        allpiecesa8h1:=allpiecesa8h1 or bit2nmaska8h1[h8];

        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[a1];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[b1];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[c1];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[d1];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[e1];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[f1];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[g1];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[h1];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[a2];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[b2];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[c2];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[d2];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[e2];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[f2];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[g2];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[h2];

        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[a7];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[b7];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[c7];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[d7];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[e7];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[f7];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[g7];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[h7];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[a8];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[b8];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[c8];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[d8];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[e8];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[f8];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[g8];
        allpiecesh8a1:=allpiecesh8a1 or bit2nmaskh8a1[h8];

        kingblack:=kingblack or bit2nmask[e8];
        kingwhite:=kingwhite or bit2nmask[e1];

        pawnwhite:=pawnwhite or bit2nmask[a2];
        pawnwhite:=pawnwhite or bit2nmask[b2];
        pawnwhite:=pawnwhite or bit2nmask[c2];
        pawnwhite:=pawnwhite or bit2nmask[d2];
        pawnwhite:=pawnwhite or bit2nmask[e2];
        pawnwhite:=pawnwhite or bit2nmask[f2];
        pawnwhite:=pawnwhite or bit2nmask[g2];
        pawnwhite:=pawnwhite or bit2nmask[h2];
        pawnblack:=pawnblack or bit2nmask[a7];
        pawnblack:=pawnblack or bit2nmask[b7];
        pawnblack:=pawnblack or bit2nmask[c7];
        pawnblack:=pawnblack or bit2nmask[d7];
        pawnblack:=pawnblack or bit2nmask[e7];
        pawnblack:=pawnblack or bit2nmask[f7];
        pawnblack:=pawnblack or bit2nmask[g7];
        pawnblack:=pawnblack or bit2nmask[h7];

        knightblack:=knightblack or bit2nmask[b8];
        knightblack:=knightblack or bit2nmask[g8];
        knightwhite:=knightwhite or bit2nmask[b1];
        knightwhite:=knightwhite or bit2nmask[g1];

        bishopblack:=bishopblack or bit2nmask[c8];
        bishopblack:=bishopblack or bit2nmask[f8];
        bishopwhite:=bishopwhite or bit2nmask[c1];
        bishopwhite:=bishopwhite or bit2nmask[f1];

        rookblack:=rookblack or bit2nmask[a8];
        rookblack:=rookblack or bit2nmask[h8];
        rookwhite:=rookwhite or bit2nmask[a1];
        rookwhite:=rookwhite or bit2nmask[h1];

        queenwhite:=queenwhite or bit2nmask[d1];
        queenblack:=queenblack or bit2nmask[d8];
        materialscore:=0;

        data.rookqueen:=data.queenblack or data.queenwhite or data.rookblack or data.rookwhite;
        data.bishopqueen:=data.queenblack or data.queenwhite or data.bishopblack or data.bishopwhite;

//        jum_perwira_putih:=7;jum_perwira_hitam:=7;
        nilai_perwira_putih:=31;nilai_perwira_hitam:=31;


        allpieces:=blackpieces or whitepieces;
        data.hashkey:=hashvalue(data,_SISIPUTIH);

{        if (data.pawnblack or data.knightblack or data.bishopblack or data.rookblack or data.queenblack or data.kingblack)<>data.blackpieces then
            begin
              inc(temp,petak_asal);
            end;
            if (data.pawnwhite or data.knightwhite or data.bishopwhite or data.rookwhite or data.queenwhite or data.kingwhite)<>data.whitepieces then
            begin
              inc(temp,petak_tujuan);
            end;
 }
   end;

end;//procedure initstate;


end.
