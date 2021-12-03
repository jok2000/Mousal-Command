
void saucer_sound()
{
	int i,f,fi,j;

	for (i=0;i<40;i++) {
		f=900;
		fi=32;
		for (j=0;j<30;j++) {
			sound(f);
			f-=fi;
			--fi;
			delay(3);
		}
		for (j=0;j<30;j++) {
			++fi;
			f+=fi;
			sound(f);
			delay(3);
		}
	}
	nosound();
}

void smart_sound()
{
	int i,f,fi,j;

	for (i=0;i<40;i++) {
		f=450;
		fi=2;
		for (j=0;j<15;j++) {
			sound(f);
			fi+=2;
			f+=fi;
			delay(6);
		}
	}
	nosound();
}

void main()
{
	saucer_sound();
}