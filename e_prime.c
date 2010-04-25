#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "weecrypt.h"

const char *e_string =
"27182818284590452353602874713526624977572470936999595749669676277240766303535"
"47594571382178525166427427466391932003059921817413596629043572900334295260595630"
"73813232862794349076323382988075319525101901157383418793070215408914993488416750"
"92447614606680822648001684774118537423454424371075390777449920695517027618386062"
"61331384583000752044933826560297606737113200709328709127443747047230696977209310"
"14169283681902551510865746377211125238978442505695369677078544996996794686445490"
"59879316368892300987931277361782154249992295763514822082698951936680331825288693"
"98496465105820939239829488793320362509443117301238197068416140397019837679320683"
"28237646480429531180232878250981945581530175671736133206981125099618188159304169"
"03515988885193458072738667385894228792284998920868058257492796104841984443634632"
"44968487560233624827041978623209002160990235304369941849146314093431738143640546"
"25315209618369088870701676839642437814059271456354906130310720851038375051011574"
"77041718986106873969655212671546889570350354021234078498193343210681701210056278"
"80235193033224745015853904730419957777093503660416997329725088687696640355570716"
"22684471625607988265178713419512466520103059212366771943252786753985589448969709"
"64097545918569563802363701621120477427228364896134225164450781824423529486363721"
"41740238893441247963574370263755294448337998016125492278509257782562092622648326"
"27793338656648162772516401910590049164499828931505660472580277863186415519565324"
"42586982946959308019152987211725563475463964479101459040905862984967912874068705"
"04895858671747985466775757320568128845920541334053922000113786300945560688166740"
"01698420558040336379537645203040243225661352783695117788386387443966253224985065"
"49958862342818997077332761717839280349465014345588970719425863987727547109629537"
"41521115136835062752602326484728703920764310059584116612054529703023647254929666"
"93811513732275364509888903136020572481765851180630364428123149655070475102544650"
"11727211555194866850800368532281831521960037356252794495158284188294787610852639"
"81395599006737648292244375287184624578036192981971399147564488262603903381441823"
"26251509748279877799643730899703888677822713836057729788241256119071766394650706"
"33045279546618550966661856647097113444740160704626215680717481877844371436988218"
"55967095910259686200235371858874856965220005031173439207321139080329363447972735"
"59552773490717837934216370120500545132638354400018632399149070547977805669785335"
"80489669062951194324730995876552368128590413832411607226029983305353708761389396"
"39177957454016137223618789365260538155841587186925538606164779834025435128439612"
"94603529133259427949043372990857315802909586313826832914771163963370924003168945"
"86360606458459251269946557248391865642097526850823075442545993769170419777800853"
"62730941710163434907696423722294352366125572508814779223151974778060569672538017"
"18077636034624592787784658506560507808442115296975218908740196609066518035165017"
"92504619501366585436632712549639908549144200014574760819302212066024330096412704"
"89439039717719518069908699860663658323227870937650226014929101151717763594460202"
"32493002804018677239102880978666056511832600436885088171572386698422422010249505"
"51881694803221002515426494639812873677658927688163598312477886520141174110913601"
"16499507662907794364600585194199856016264790761532103872755712699251827568798930"
"27617611461625493564959037980458381823233686120162437365698467037858533052758333"
"37939907521660692380533698879565137285593883499894707416181550125397064648171946"
"70834819721448889879067650379590366967249499254527903372963616265897603949857674"
"13973594410237443297093554779826296145914429364514286171585873397467918975712119"
"56187385783644758448423555581050025611492391518893099463428413936080383091662818"
"81150371528496705974162562823609216807515017772538740256425347087908913729172282"
"86115159156837252416307722544063378759310598267609442032619242853170187817729602"
"35413060672136046000389661093647095141417185777014180606443636815464440053316087"
"78314317444081194942297559931401188868331483280270655383300469329011574414756313"
"99972217038046170928945790962716622607407187499753592127560844147378233032703301"
"68237193648002173285734935947564334129943024850235732214597843282641421684878721"
"67336701061509424345698440187331281010794512722373788612605816566805371439612788"
"87325273738903928905068653241380627960259303877276977837928684093253658807339884"
"57218746021005311483351323850047827169376218004904795597959290591655470505777514"
"30817511269898518840871856402603530558373783242292418562564425502267215598027401"
"26179719280471396006891638286652770097527670697770364392602243728418408832518487"
"70472638440379530166905465937461619323840363893131364327137688841026811219891275"
"22305625675625470172508634976536728860596675274086862740791285657699631378975303"
"46606166698042182677245605306607738996242183408598820718646826232150802882863597"
"46839654358856685503773131296587975810501214916207656769950659715344763470320853"
"21560367482860837865680307306265763346977429563464371670939719306087696349532884"
"68336130388294310408002968738691170666661468000151211434422560238744743252507693"
"87077775193299942137277211258843608715834835626961661980572526612206797540621062"
"08064988291845439530152998209250300549825704339055357016865312052649561485724925"
"73862069174036952135337325316663454665885972866594511364413703313936721185695539"
"52108458407244323835586063106806964924851232632699514603596037297253198368423363"
"90463213671011619282171115028280160448805880238203198149309636959673583274202498"
"82456849412738605664913525267060462344505492275811517093149218795927180019409688"
"66986837037302200475314338181092708030017205935530520700706072233999463990571311"
"58709963577735902719628506114651483752620956534671329002599439766311454590268589"
"89791158370934193704411551219201171648805669459381311838437656206278463104903462"
"93950029458341164824114969758326011800731699437393506966295712410273239138741754"
"92307186245454322203955273529524024590380574450289224688628533654221381572213116"
"32881120521464898051800920247193917105553901139433166815158288436876069611025051"
"71007392762385553386272553538830960671644662370922646809671254061869502143176211"
"66814009759528149390722260111268115310838731761732323526360583817315103459573653"
"82235349929358228368510078108846343499835184044517042701893819942434100905753762"
"57767571118090088164183319201962623416288166521374717325477727783488774366518828"
"75215668571950637193656539038944936642176400312152787022236646363575550356557694"
"88865495002708539236171055021311474137441061344455441921013361729962856948991933"
"69184729478580729156088510396781959429833186480756083679551496636448965592948187"
"85178403877332624705194505041984774201418394773120281588684570729054405751060128"
"52580565947030468363445926525521370080687520095934536073162261187281739280746230"
"94685367823106097921599360019946237993434210687813497346959246469752506246958616"
"90917857397659519939299399556754271465491045686070209901260681870498417807917392"
"40719459963230602547079017745275131868099822847308607665368668555164677029113368"
"27563107223346726113705490795365834538637196235856312618387156774118738527722922"
"59474337378569553845624680101390572787101651296663676445187246565373040244368414"
"08144887329578473484900030194778880204603246608428753518483649591950828883232065"
"22128104190448047247949291342284951970022601310430062410717971502793433263407995"
"96053144605323048852897291765987601666781193793237245385720960758227717848336161"
"35826128962261181294559274627671377944875867536575448614076119311259585126557597"
"34573015333642630767985443385761715333462325270572005303988289499034259566232975"
"78248873502925916682589445689465599265845476269452878051650172067478541788798227"
"68065366506419109734345288783386217261562695826544782056729877564263253215942944"
"18039943217000090542650763095588465895171709147607437136893319469090981904501290"
"30709956622662030318264936573369841955577696378762491885286568660760056602560544"
"57113372868402055744160308370523122425872234388541231794813885500756893811249353"
"86318635287083799845692619981794523364087429591180747453419551420351726184200845"
"50917084568236820089773945584267921427347756087964427920270831215015640634134161"
"71664480698154837644915739001212170415478725919989438253649505147713793991472052"
"19529079396137621107238494290616357604596231253506068537651423115349665683715116"
"60422079639446662116325515772907097847315627827759878813649195125748332879377157"
"14590910648416426783099497236744201758622694021594079244805412553604313179926967"
"39157542419296607312393763542139230617876753958711436104089409966089471418340698"
"36299367536262154524729846421375289107988438130609555262272083751862983706678722"
"44301957937937860721072542772890717328548743743557819665117166183308811291202452"
"04048682200072344035025448202834254187884653602591506445271657700044521097735585"
"89762265548494162171498953238342160011406295071849042778925855274303522139683567"
"90180764060421383073087744601708426882722611771808426643336517800021719034492342"
"64266292261456004337383868335555343453004264818473989215627086095650629340405264"
"94324426144566592129122564889356965500915430642613425266847259491431423939884543"
"24863274618428466559853323122104662598901417121034460842716166190012571958707932"
"17569698544013397622096749454185407118446433946990162698351607848924514058940946"
"39526780735457970030705116368251948770118976400282764841416058720618418529718915"
"40196882532893091496653457535714273184820163846448324990378860690080727093276731"
"27581966563941148961716832980455139729506687604740915420428429993541025829113502"
"24169076943166857424252250902693903481485645130306992519959043638402842926741257"
"34224477655841778861717372654620854982944989467873509295816526320722589923687684"
"57017823038096567883112289305809140572610865884845873101658151167533327674887014"
"82916741970151255978257270740643180860142814902414678047232759768426963393577354"
"29301867394397163886117642090040686633988568416810038723892144831760701166845038"
"87212364367043314091155733280182977988736590916659612402021778558854876176161989"
"37079438005666336488436508914480557103976521469602766258359905198704230017946553"
"67885674302859746001437854832370687011900784994049309189191816493272597740300748"
"79681484882342932023012128032327460392219687528340516906974194257614673978110715"
"46418627336909158497318501118396048253351874843892317729261354302493256289637136"
"19772854566229244616444972845978677115741256703078718851093363444801496752406185"
"36569532074170533486782754827815415561966911055101472799040386897220465550833170"
"78239480878599050194756310898412414467282186545997159663901564194175182093593261"
"63168883801327587526014605076760983926257264111201352885913178482994756824725648"
"85533357279772205543568126302535748216585414000805314820697137262149755576051890"
"48162237679041492674260007104592269531483518813746388710427354476762357793399397"
"06323966049691453032738878745579059349377723201429548033450006952569809352828877"
"83710670585567749481373858630385762823040694005665340584887527005308832459182183"
"49431804983419963998145877343586311594057044368351528538360944295596436067609022"
"17418968835481316439974377641583652422346426195973904554506806952328507518687194"
"49064767791886720306418630751053512149851051207313846648717547518382979990189317"
"75155063998101646641459210240683829460320853555405814715927322067756766921366408"
"15059008069525406106285364082932766219319399338616238360691117677854482361293268"
"58199965239275488427435414402884536455595124735546139403154952097397051896240157"
"97683263945063323045219264504965173546677569929571898969047090273028854494541669"
"97919929480382549802859460290527631455803165140662291712234293758061439934849143"
"62107993576737317948964252488813720435579287511385856973381976083524423240466778"
"02094839963994668483377470672548361884827300064831916382602211055522124673332318"
"44630055044818499169966220877461402161570210296033185887273332987793525701823938"
"61244026868339555870607758169954398469568540671174444932479519572159419645863736"
"12691552645757478698596424217659289686238350637043393981167139754473622862550680"
"36826641355414480489977213731741191999700172939073033508690209225191244473932783"
"76156321810842898207706974138707053266117683698647741787180202729412982310888796"
"83188085436732780687977165911165422445380662586171172949803824887998650406156397"
"56299369628093581897614910171453435566595427570641944088338168411111662007597872"
"44137082333917886114708228657531078536674695018462140736493917366254937783014074"
"30266842215033511773647185387232404042103790775026602011481493548222891666364078"
"24501668153412135052785785393326061102498022730936367402135153864316930152674605"
"36064351732154701091440650878823636764236831187390937464232609021646365627553976"
"83401948293279575062439964527257862440037598342205080893512902312247597064410567"
"83618708771723335554654825989068612014101072224659040085537982352538851716235182"
"56518482203125214950700378300411216212126052726059944320443056274522916128891766"
"81416063913123597535039032007752958739241247645185080916391145929607115634420434"
"71335447209811784614510778723991406062902282766643092649005922498102910687594345"
"33858330391178747575977065953570979640012224092199031158229259667913153991561438"
"07012926078019702258966292336815431249941225946002339947222817105660393187722680"
"04938331489803385489094686851307892920642428191747958661999444111962087304980643"
"85006852620258432842085582338566936649849720817046135376163584015342840674118587"
"58154651459827022867667185530931192334019128617061336487318319756081256946008940"
"29530944291195902959685639230376899763274622839007354571445964141082292859222393"
"32836210192822937243590283003884445701383771632056518351970100115722010956997890"
"48496445343461212922496473235612632195115570156582442766159932646315580667205312"
"75969485380573642083849188870951760522878173394627476446568589009362661233111529"
"10816041524100214195937349786431661556732702792109593543055579732660554677963552"
"00537830461954063697184291616858273412221714588587081427409024818544642177487692"
"50933287856706746773812267528316535592452045780705413525769032535227389638474956"
"46255940378924925007624386893776475310102323746733771474581625530698032499033676"
"45543030527456151296121458594443215074905149145395098100138873792637996487372839"
"64168975551322759620118382486507469854920380976919326064376087432093856028156428"
"49756549307909733854185583515789409814007691892389063090542534883896831762904120"
"21294916719581193579120316251434409650313283521672802137241594734409549831613832"
"25054867081722214751384251667904454166173032008203309028954888085167972584958134"
"07132180533988828139346049850532340472595097214331492586604248511405819579711564"
"19145884283300052568477687430591639049430687134311879618963747550336282093994934"
"36903210319768981120555953694654247041733238953940460353253967583543953505167202"
"61647961347790912327995264929045151148307923369382166010702872651938143844844532"
"63951739411013115250275046574934306376654186612891526444692622288436629946273246"
"79587363835019371427864713980540382155134632237020715331348870831741465914924063"
"59493020921122052610312390682941345696785958518393491382340884274312419099152870"
"80433280913299307893686712741392289003306999587592181529761248240911695158778996"
"40903525773459382482320530555672380950222667904396142318529919891810655544124772"
"04508510210071522352342792531266930108270633942321762570076323139159349709946933"
"24101390877916165122680441480976561897973504315139606691325837903374862083669547"
"50832803187867077511775256639634792592197335779495554986552141933981702686399873"
"88347010255262052312317215254062571636771270010760912281528326508984359568975961"
"03837215772683117073455225019412170154131879365181850202087732690613359218200076"
"23272695032838273912438281981708711681089511878967467070733778695925655427133400"
"52326706040004348843432902760360498027862160749469654989210474443927871934536701"
"79867392080384563372331198385586263800851634559719444199434462476112384461761573"
"62420159350785208256006041015568898995017325543372980735616998611019084720966007"
"08320280569917042590103876928658336557728758684250492690370934262028022399861803"
"40021132074219864291738367917623282644464575633033655677737480864410996914182777"
"42534170109884358531893391759345115740238472929090154685591637926961968410006765"
"98399744972047287881831200233383298030567865480871476464512824264478216644266616"
"73209601256479451482712567132669706736714461779564375239174292850398702258373406"
"98523091904649672602434112703456111141498357839017934997137909136967064976371272"
"48466613279908254305449295528594932793818341607827091326680865655921102733746700"
"13258342871524083566152216557499843123627828710664940156467014194371382386345472"
"96069786933359731095371264994162826564637084905801515382053383265112895049385664"
"68752921135932220265681856418260827538790002407915892646028490894922299966167437"
"73134777613415096526244833270934389841205692614510885781224913961691253420291813"
"98986839013357958576244351940089439551805547465540000517662402028259448288338118"
"86381749594284892013520090951007864941868256009273977667585642598378587497776669"
"56335017074857902724870137026420328396575634801081835618237217708223642318659159"
"58836694873224117265044872683923284530109916775183768315998212632371238543573126"
"81202445175401852132663740538802901249728180895021553100673598184430429105288459"
"32306472559044235596055197883932593033957293466305516043092378567722929353720841"
"66931345752840118737468546916206489911647269094289829710656068018058078436004618"
"66223562874591385185904416250663222249561448724413813849763797102676020845531824"
"11196392794106961946542648000676172761811563006364432111622483737910562361135883"
"63345501022861705178904405704195778598333484633179219044946529230214692597565663"
"89965893747728751393377105569802455757436190501772466214587592374418657530064998"
"05668837696422982550119506583784312523213530937123524396914966231011032824357006"
"57814876772991609411539540633627524237129355499267134850315782388995675452879155"
"78420483105749330060197958207739558522807307048950936235550769837881926357141779"
"33875021634439101418757671193891441627710960285941580971991342931329514592437363"
"64564730350373745385034892861131416380947523017450887848856457412750033533034161"
"38096560043105860548355773946625033230034341587814634602169235079216111013148948"
"28189539102891681632870930971318413981542767881806762865097808571826211700314000"
"33773015815363341490932370347036375133545376345210503709954529420552320788174493"
"70937677056009306353645510913481627378204985657055608784211964039972344556458607"
"68951556968689938489643919522523230970330103727722771087056491296612106149407278"
"24420334140574414464599682369661188784116562903551178399440709617725671649197901"
"68195234523807446299877664824873753313018142763910519234685081979001796519907050"
"49086523744284165277661142535153866516278131609096480280123449337242786693089482"
"79134654439319652541548294945778757585994820991818245224493120777682508307682823"
"35001597040419199560509705364696473142448453825888112602753909548852639708652339"
"05294182969180235712054532823180927035649174337193208062873130358964057087377996"
"78451747405153174013848780828810060463889367116404777559854812639075047472950126"
"09419990373721246201677030517790352952793168766305099837441859803498821239340919"
"80505510382153982767729137313800671533924012695458637642206509781085290763907972"
"78413017645532475270737887640693664200121947457023582954813657818098679440202202"
"80822637957006755393575808086318932075864444206644691649334467698180811716568665"
"21338968617359245092080146531252977796613719869591645186943232424640440167238197"
"80207283944182645021831314833660193848919723178171543721921039466384737156302267"
"01801343515930442853848941825678870721238520597263859224934763623122188113706307"
"50691826010968906925141714251421815349153212907772374850663548917089285076023435"
"17682183550088296474106558148820492395337022705367056307503174997881870099892510"
"20178015601042277836283644323729779929935160925884515772055232896978333126427671"
"29109399310377342591059230327765266764187484244107656444776709779039232495841634"
"85277351719810646738371427429744689923204069325060628344689375430167878153206160"
"09057693404906146176607094380110915443261929000745209895959201159412324102274845"
"48260540436187183633026899285862358214564387969521023526667337243442309157718327"
"75658002119282703910423919664269111553335945696857828170203254955525288754644660"
"74620294766116004435551604735044292127916358748473501590215522120388281168021413"
"86586516846456996481001563374125509847973013865627546016127924635978366148016387"
"16027944054827101962907745436280926125675071817736417497632544367735036325800040"
"42919906963117397787875081560227368824967077635559869284901628768699628053790181"
"84814881083394690001638079107596074550468891268679281239114888003672072973080135"
"44313253477130941867171786075229813735391267728125939582205242899913716906856504"
"21575056729991274177149279608831502358697816190894908487717722503860872618384947";

#define NDIGITS	10

int
main(void)
{
	mp_digit *n;
	mp_digit factor, remainder;
	mp_size len;
	char buf[NDIGITS+1];

	int i, e_len = strlen(e_string);
	for (i=0; i<e_len-NDIGITS; i++) {
		memcpy(buf, &e_string[i], NDIGITS);
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
