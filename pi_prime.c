#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "weecrypt.h"

const char *pi_string =
"3"

"14159265358979323846264338327950288419716939937510"
"58209749445923078164062862089986280348253421170679"
"82148086513282306647093844609550582231725359408128"
"48111745028410270193852110555964462294895493038196"
"44288109756659334461284756482337867831652712019091"
"45648566923460348610454326648213393607260249141273"
"72458700660631558817488152092096282925409171536436"
"78925903600113305305488204665213841469519415116094"
"33057270365759591953092186117381932611793105118548"
"07446237996274956735188575272489122793818301194912"
"98336733624406566430860213949463952247371907021798"
"60943702770539217176293176752384674818467669405132"
"00056812714526356082778577134275778960917363717872"
"14684409012249534301465495853710507922796892589235"
"42019956112129021960864034418159813629774771309960"
"51870721134999999837297804995105973173281609631859"
"50244594553469083026425223082533446850352619311881"
"71010003137838752886587533208381420617177669147303"
"59825349042875546873115956286388235378759375195778"
"18577805321712268066130019278766111959092164201989"

"38095257201065485863278865936153381827968230301952"
"03530185296899577362259941389124972177528347913151"
"55748572424541506959508295331168617278558890750983"
"81754637464939319255060400927701671139009848824012"
"85836160356370766010471018194295559619894676783744"
"94482553797747268471040475346462080466842590694912"
"93313677028989152104752162056966024058038150193511"
"25338243003558764024749647326391419927260426992279"
"67823547816360093417216412199245863150302861829745"
"55706749838505494588586926995690927210797509302955"
"32116534498720275596023648066549911988183479775356"
"63698074265425278625518184175746728909777727938000"
"81647060016145249192173217214772350141441973568548"
"16136115735255213347574184946843852332390739414333"
"45477624168625189835694855620992192221842725502542"
"56887671790494601653466804988627232791786085784383"
"82796797668145410095388378636095068006422512520511"
"73929848960841284886269456042419652850222106611863"
"06744278622039194945047123713786960956364371917287"
"46776465757396241389086583264599581339047802759009"

"94657640789512694683983525957098258226205224894077"
"26719478268482601476990902640136394437455305068203"
"49625245174939965143142980919065925093722169646151"
"57098583874105978859597729754989301617539284681382"
"68683868942774155991855925245953959431049972524680"
"84598727364469584865383673622262609912460805124388"
"43904512441365497627807977156914359977001296160894"
"41694868555848406353422072225828488648158456028506"
"01684273945226746767889525213852254995466672782398"
"64565961163548862305774564980355936345681743241125"
"15076069479451096596094025228879710893145669136867"
"22874894056010150330861792868092087476091782493858"
"90097149096759852613655497818931297848216829989487"
"22658804857564014270477555132379641451523746234364"
"54285844479526586782105114135473573952311342716610"
"21359695362314429524849371871101457654035902799344"
"03742007310578539062198387447808478489683321445713"
"86875194350643021845319104848100537061468067491927"
"81911979399520614196634287544406437451237181921799"
"98391015919561814675142691239748940907186494231961"

"56794520809514655022523160388193014209376213785595"
"66389377870830390697920773467221825625996615014215"
"03068038447734549202605414665925201497442850732518"
"66600213243408819071048633173464965145390579626856"
"10055081066587969981635747363840525714591028970641"
"40110971206280439039759515677157700420337869936007"
"23055876317635942187312514712053292819182618612586"
"73215791984148488291644706095752706957220917567116"
"72291098169091528017350671274858322287183520935396"
"57251210835791513698820914442100675103346711031412"
"67111369908658516398315019701651511685171437657618"
"35155650884909989859982387345528331635507647918535"
"89322618548963213293308985706420467525907091548141"
"65498594616371802709819943099244889575712828905923"
"23326097299712084433573265489382391193259746366730"
"58360414281388303203824903758985243744170291327656"
"18093773444030707469211201913020330380197621101100"
"44929321516084244485963766983895228684783123552658"
"21314495768572624334418930396864262434107732269780"
"28073189154411010446823252716201052652272111660396"

"66557309254711055785376346682065310989652691862056"
"47693125705863566201855810072936065987648611791045"
"33488503461136576867532494416680396265797877185560"
"84552965412665408530614344431858676975145661406800"
"70023787765913440171274947042056223053899456131407"
"11270004078547332699390814546646458807972708266830"
"63432858785698305235808933065757406795457163775254"
"20211495576158140025012622859413021647155097925923"
"09907965473761255176567513575178296664547791745011"
"29961489030463994713296210734043751895735961458901"
"93897131117904297828564750320319869151402870808599"
"04801094121472213179476477726224142548545403321571"
"85306142288137585043063321751829798662237172159160"
"77166925474873898665494945011465406284336639379003"
"97692656721463853067360965712091807638327166416274"
"88880078692560290228472104031721186082041900042296"
"61711963779213375751149595015660496318629472654736"
"42523081770367515906735023507283540567040386743513"
"62222477158915049530984448933309634087807693259939"
"78054193414473774418426312986080998886874132604721"

"56951623965864573021631598193195167353812974167729"
"47867242292465436680098067692823828068996400482435"
"40370141631496589794092432378969070697794223625082"
"21688957383798623001593776471651228935786015881617"
"55782973523344604281512627203734314653197777416031"
"99066554187639792933441952154134189948544473456738"
"31624993419131814809277771038638773431772075456545"
"32207770921201905166096280490926360197598828161332"
"31666365286193266863360627356763035447762803504507"
"77235547105859548702790814356240145171806246436267"
"94561275318134078330336254232783944975382437205835"
"31147711992606381334677687969597030983391307710987"
"04085913374641442822772634659470474587847787201927"
"71528073176790770715721344473060570073349243693113"
"83504931631284042512192565179806941135280131470130"
"47816437885185290928545201165839341965621349143415"
"95625865865570552690496520985803385072242648293972"
"85847831630577775606888764462482468579260395352773"
"48030480290058760758251047470916439613626760449256"
"27420420832085661190625454337213153595845068772460";

#define NDIGITS	10

int
main(void)
{
	mp_digit *n;
	mp_digit factor, remainder;
	mp_size len;
	char buf[NDIGITS+1];

	int i, pilen = strlen(pi_string);
	for (i=0; i<pilen-NDIGITS; i++) {
		memcpy(buf, &pi_string[i], NDIGITS);
		buf[NDIGITS] = 0;

		n = mp_from_str(buf, 10, &len);
		if (n == 0 || len == 0 || (len == 1 && n[0]<=2)) {
			printf("Invalid number \"%s\"\n", buf);
			exit(1);
		}

		if (mp_sieve(n, len, 0) != 0)
			continue;

		if (len == 1 && n[0] <= 2) {
			continue;
		} else {
			/* Now run 20 rounds of the Rabin-Miller test. */
			if (!mp_composite(n, len, 20)) {
				mp_print_dec(n, len);
				printf(" is prime with extreme probability (offset %d).\n", i);
			}
		}
	}

	return 0;
}
