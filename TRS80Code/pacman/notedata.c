

typedef struct {
	unsigned char name[3];
	unsigned char octave;
	unsigned char b1;
	unsigned char b2; 
}
noteData;

noteData notes[73];

void initNoteData() {
    int octave = 0;
    for(int i=0;i<72;i=i+12) {
		strcpy(notes[i].name, "A");
		notes[i].octave = octave;
		strcpy(notes[1+i].name, "A#");
		notes[1+i].octave = octave;
		strcpy(notes[2+i].name, "B");
		notes[2+i].octave = octave;
		strcpy(notes[3+i].name, "C");
		notes[3+i].octave = octave;
		strcpy(notes[4+i].name, "C#");
		notes[4+i].octave = octave;
		strcpy(notes[5+i].name, "D");
		notes[5+i].octave = octave;
		strcpy(notes[6+i].name, "D#");
		notes[6+i].octave = octave;
		strcpy(notes[7+i].name, "E");
		notes[7+i].octave = octave;
		strcpy(notes[8+i].name, "F");
		notes[8+i].octave = octave;
		strcpy(notes[9+i].name,"F#");
		notes[9+i].octave = octave;
		strcpy(notes[10+i].name,"G");
		notes[10+i].octave = octave;
		strcpy(notes[11+i].name, "G#");
		notes[11+i].octave = octave;
		octave++;
	}

	notes[0].b1=81;    notes[0].b2=252;
	notes[1].b1=17;    notes[1].b2=60;
	notes[2].b1=113;   notes[2].b2=28;
	notes[3].b1=49;    notes[3].b2=172;
	notes[4].b1=81;    notes[4].b2=76;
	notes[5].b1=177;   notes[5].b2=244;
	notes[6].b1=1;     notes[6].b2=180;
	notes[7].b1=17;    notes[7].b2=84;
	notes[8].b1=1;     notes[8].b2=20;
	notes[9].b1=177;   notes[9].b2=164;
	notes[10].b1=209;  notes[10].b2=196;
	notes[11].b1=49;   notes[11].b2=132;
	notes[12].b1=113;  notes[12].b2=248;
	notes[13].b1=1;    notes[13].b2=120;
	notes[14].b1=225;  notes[14].b2=56;
	notes[15].b1=145;  notes[15].b2=88;
	notes[16].b1=161;  notes[16].b2=152;
	notes[17].b1=241;  notes[17].b2=232;
	notes[18].b1=225;  notes[18].b2=104;
	notes[19].b1=33;   notes[19].b2=168;
	notes[20].b1=1;    notes[20].b2=40;
	notes[21].b1=113;  notes[21].b2=72;
	notes[22].b1=113;  notes[22].b2=136;
	notes[23].b1=113;  notes[23].b2=8;
	notes[24].b1=113;  notes[24].b2=240;
	notes[25].b1=1;    notes[25].b2=240;
	notes[26].b1=193;  notes[26].b2=112;
	notes[27].b1=97;   notes[27].b2=176;
	notes[28].b1=81;   notes[28].b2=48;
	notes[29].b1=241;  notes[29].b2=208;
	notes[30].b1=193;  notes[30].b2=208;
	notes[31].b1=81;   notes[31].b2=80;
	notes[32].b1=1;    notes[32].b2=80;
	notes[33].b1=17;   notes[33].b2=144;
	notes[34].b1=241;  notes[34].b2=16;
	notes[35].b1=225;  notes[35].b2=16;
	notes[36].b1=241;  notes[36].b2=224;
	notes[37].b1=17;   notes[37].b2=224;
	notes[38].b1=129;  notes[38].b2=224;
	notes[39].b1=209;  notes[39].b2=96;
	notes[40].b1=161;  notes[40].b2=96;
	notes[41].b1=1;    notes[41].b2=96;
	notes[42].b1=81;   notes[42].b2=160;
	notes[43].b1=161;  notes[43].b2=160;
	notes[44].b1=1;    notes[44].b2=160;
	notes[45].b1=49;   notes[45].b2=32;
	notes[46].b1=17;   notes[46].b2=32;
	notes[47].b1=193;  notes[47].b2=32;
	notes[48].b1=1;    notes[48].b2=32;
	notes[49].b1=49;   notes[49].b2=192;
	notes[50].b1=17;   notes[50].b2=192;
	notes[51].b1=161;  notes[51].b2=192;
	notes[52].b1=193;  notes[52].b2=192;
	notes[53].b1=1;    notes[53].b2=192;
	notes[54].b1=177;  notes[54].b2=64;
	notes[55].b1=81;   notes[55].b2=64;
	notes[56].b1=17;   notes[56].b2=64;
	notes[57].b1=96;   notes[57].b2=64;
	notes[58].b1=33;   notes[58].b2=64;
	notes[59].b1=129;  notes[59].b2=64;
	notes[60].b1=1;    notes[60].b2=64;
	notes[61].b1=113;  notes[61].b2=128;
	notes[62].b1=49;   notes[62].b2=128;
	notes[63].b1=81;   notes[63].b2=128;
	notes[64].b1=145;  notes[64].b2=128;
	notes[65].b1=17;   notes[65].b2=128;
	notes[66].b1=97;   notes[66].b2=128;
	notes[67].b1=161;  notes[67].b2=128;
	notes[68].b1=255;  notes[68].b2=255;
	notes[69].b1=255;  notes[69].b2=255;
	notes[70].b1=255;  notes[70].b2=255;
	notes[71].b1=255;  notes[71].b2=255;
	notes[72].b1=255;  notes[72].b2=255;
}