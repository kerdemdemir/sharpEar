/*
  Copyright (C) 2003-2009 Paul Brossier <piem@aubio.org>

  This file is part of aubio.

  aubio is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  aubio is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with aubio.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "aubio_priv.h"
#include "types.h"
#include "fvec.h"
#include "lvec.h"
#include "temporal/filter.h"
#include "temporal/a_weighting.h"

uint_t
aubio_filter_set_a_weighting (aubio_filter_t * f, uint_t samplerate)
{
  uint_t order; lsmp_t *a, *b; lvec_t *as, *bs;
  aubio_filter_set_samplerate (f, samplerate);
  bs = aubio_filter_get_feedforward (f);
  as = aubio_filter_get_feedback (f);
  b = bs->data, a = as->data;
  order = aubio_filter_get_order (f);

  if (order != 7) {
    AUBIO_ERROR ("order of A-weighting filter must be 7, not %d\n", order);
    return 1;
  }

  /* select coefficients according to sampling frequency */
  switch (samplerate) {

    case 8000:
      b[0] =  6.306209468238731519207362907764036208391189575195312500e-01;
      b[1] = -1.261241893647746525886077506584115326404571533203125000e+00;
      b[2] = -6.306209468238730408984338282607495784759521484375000000e-01;
      b[3] =  2.522483787295493051772155013168230652809143066406250000e+00;
      b[4] = -6.306209468238730408984338282607495784759521484375000000e-01;
      b[5] = -1.261241893647746525886077506584115326404571533203125000e+00;
      b[6] =  6.306209468238731519207362907764036208391189575195312500e-01;
      a[0] =  1.000000000000000000000000000000000000000000000000000000e+00;
      a[1] = -2.128467193009123015201566886389628052711486816406250000e+00;
      a[2] =  2.948668980101234460278192273108288645744323730468750000e-01;
      a[3] =  1.824183830735050637628091863007284700870513916015625000e+00;
      a[4] = -8.056628943119792385374466903158463537693023681640625000e-01;
      a[5] = -3.947497982842933517133587884018197655677795410156250000e-01;
      a[6] =  2.098548546080332977137317129745497368276119232177734375e-01;
      break;

    case 11025:
      b[0] =  6.014684165832374640459079273568931967020034790039062500e-01;
      b[1] = -1.202936833166475150136420779745094478130340576171875000e+00;
      b[2] = -6.014684165832373530236054648412391543388366699218750000e-01;
      b[3] =  2.405873666332950300272841559490188956260681152343750000e+00;
      b[4] = -6.014684165832373530236054648412391543388366699218750000e-01;
      b[5] = -1.202936833166475150136420779745094478130340576171875000e+00;
      b[6] =  6.014684165832374640459079273568931967020034790039062500e-01;
      a[0] =  1.000000000000000000000000000000000000000000000000000000e+00;
      a[1] = -2.463578747722854345170162559952586889266967773437500000e+00;
      a[2] =  1.096799662705210121060872552334330976009368896484375000e+00;
      a[3] =  1.381222210556041218865175324026495218276977539062500000e+00;
      a[4] = -1.013875696476876031582037285261321812868118286132812500e+00;
      a[5] = -1.839132734476921215982514468123554252088069915771484375e-01;
      a[6] =  1.833526393172056623281918064094497822225093841552734375e-01;
      break;

    case 16000:
      b[0] =  5.314898298235570806014038680586963891983032226562500000e-01;
      b[1] = -1.062979659647114161202807736117392778396606445312500000e+00;
      b[2] = -5.314898298235570806014038680586963891983032226562500000e-01;
      b[3] =  2.125959319294228322405615472234785556793212890625000000e+00;
      b[4] = -5.314898298235570806014038680586963891983032226562500000e-01;
      b[5] = -1.062979659647114161202807736117392778396606445312500000e+00;
      b[6] =  5.314898298235570806014038680586963891983032226562500000e-01;
      a[0] =  1.000000000000000000000000000000000000000000000000000000e+00;
      a[1] = -2.867832572992162987191022693878039717674255371093750000e+00;
      a[2] =  2.221144410202312347024644623161293566226959228515625000e+00;
      a[3] =  4.552683347886614662058946123579517006874084472656250000e-01;
      a[4] = -9.833868636162828025248927588108927011489868164062500000e-01;
      a[5] =  5.592994142413361402521587706360151059925556182861328125e-02;
      a[6] =  1.188781038285612462468421313133148942142724990844726562e-01;
      break;

    case 22050:
      b[0] =  4.492998504299193784916610638902056962251663208007812500e-01;
      b[1] = -8.985997008598388680056245902960654348134994506835937500e-01;
      b[2] = -4.492998504299192674693586013745516538619995117187500000e-01;
      b[3] =  1.797199401719677958055854105623438954353332519531250000e+00;
      b[4] = -4.492998504299192674693586013745516538619995117187500000e-01;
      b[5] = -8.985997008598388680056245902960654348134994506835937500e-01;
      b[6] =  4.492998504299193784916610638902056962251663208007812500e-01;
      a[0] =  1.000000000000000000000000000000000000000000000000000000e+00;
      a[1] = -3.229078805225074955131958631682209670543670654296875000e+00;
      a[2] =  3.354494881236033787530459449044428765773773193359375000e+00;
      a[3] = -7.317843680657351024265722116979304701089859008789062500e-01;
      a[4] = -6.271627581807257545420952737913466989994049072265625000e-01;
      a[5] =  1.772142005020879151899748649157118052244186401367187500e-01;
      a[6] =  5.631716697383508385410522123493137769401073455810546875e-02;
      break;

    case 24000:
      b[0] =  4.256263892891054001488271296693710610270500183105468750e-01;
      b[1] = -8.512527785782106892753517968230880796909332275390625000e-01;
      b[2] = -4.256263892891054556599783609271980822086334228515625000e-01;
      b[3] =  1.702505557156421378550703593646176159381866455078125000e+00;
      b[4] = -4.256263892891054556599783609271980822086334228515625000e-01;
      b[5] = -8.512527785782106892753517968230880796909332275390625000e-01;
      b[6] =  4.256263892891054001488271296693710610270500183105468750e-01;
      a[0] =  1.000000000000000000000000000000000000000000000000000000e+00;
      a[1] = -3.325996004241962733516402295208536088466644287109375000e+00;
      a[2] =  3.677161079286316969216841243905946612358093261718750000e+00;
      a[3] = -1.106476076828482035807610373012721538543701171875000000e+00;
      a[4] = -4.726706734908718843257702246773988008499145507812500000e-01;
      a[5] =  1.861941760230954034938122276798821985721588134765625000e-01;
      a[6] =  4.178771337829546850262119050967157818377017974853515625e-02;
      break;

    case 32000:
      b[0] =  3.434583386824304196416335344110848382115364074707031250e-01;
      b[1] = -6.869166773648609503055695313378237187862396240234375000e-01;
      b[2] = -3.434583386824303641304823031532578170299530029296875000e-01;
      b[3] =  1.373833354729721900611139062675647437572479248046875000e+00;
      b[4] = -3.434583386824303641304823031532578170299530029296875000e-01;
      b[5] = -6.869166773648609503055695313378237187862396240234375000e-01;
      b[6] =  3.434583386824304196416335344110848382115364074707031250e-01;
      a[0] =  1.000000000000000000000000000000000000000000000000000000e+00;
      a[1] = -3.656446043233668063976438133977353572845458984375000000e+00;
      a[2] =  4.831468450652579349480220116674900054931640625000000000e+00;
      a[3] = -2.557597496581567764195597192156128585338592529296875000e+00;
      a[4] =  2.533680394205302666144064005493419244885444641113281250e-01;
      a[5] =  1.224430322452567110325105659285327419638633728027343750e-01;
      a[6] =  6.764072168342137418572956875095769646577537059783935547e-03;
      break;

    case 44100:
      b[0] =  2.557411252042575133813784304948057979345321655273437500e-01;
      b[1] = -5.114822504085150267627568609896115958690643310546875000e-01;
      b[2] = -2.557411252042575133813784304948057979345321655273437500e-01;
      b[3] =  1.022964500817030053525513721979223191738128662109375000e+00;
      b[4] = -2.557411252042575133813784304948057979345321655273437500e-01;
      b[5] = -5.114822504085150267627568609896115958690643310546875000e-01;
      b[6] =  2.557411252042575133813784304948057979345321655273437500e-01;
      a[0] =  1.000000000000000000000000000000000000000000000000000000e+00;
      a[1] = -4.019576181115832369528106937650591135025024414062500000e+00;
      a[2] =  6.189406442920693862674852425698190927505493164062500000e+00;
      a[3] = -4.453198903544116404873420833609998226165771484375000000e+00;
      a[4] =  1.420842949621876627475103305187076330184936523437500000e+00;
      a[5] = -1.418254738303044160119270600262098014354705810546875000e-01;
      a[6] =  4.351177233495117681327801761881346465088427066802978516e-03;
      break;

    case 48000:
      b[0] =  2.343017922995132285013397677175817079842090606689453125e-01;
      b[1] = -4.686035845990265125138307666929904371500015258789062500e-01;
      b[2] = -2.343017922995132007457641520886681973934173583984375000e-01;
      b[3] =  9.372071691980530250276615333859808743000030517578125000e-01;
      b[4] = -2.343017922995132007457641520886681973934173583984375000e-01;
      b[5] = -4.686035845990265125138307666929904371500015258789062500e-01;
      b[6] =  2.343017922995132285013397677175817079842090606689453125e-01;
      a[0] =  1.000000000000000000000000000000000000000000000000000000e+00;
      a[1] = -4.113043408775872045168853219365701079368591308593750000e+00;
      a[2] =  6.553121752655050258340452273841947317123413085937500000e+00;
      a[3] = -4.990849294163385074796224216697737574577331542968750000e+00;
      a[4] =  1.785737302937575599059982778271660208702087402343750000e+00;
      a[5] = -2.461905953194876706113802811159985139966011047363281250e-01;
      a[6] =  1.122425003323123879339640041052916785702109336853027344e-02;
      break;

    case 88200:
      b[0] =  1.118876366882113199130444058937428053468465805053710938e-01;
      b[1] = -2.237752733764226120705131961585721001029014587402343750e-01;
      b[2] = -1.118876366882113337908322137081995606422424316406250000e-01;
      b[3] =  4.475505467528452241410263923171442002058029174804687500e-01;
      b[4] = -1.118876366882113337908322137081995606422424316406250000e-01;
      b[5] = -2.237752733764226120705131961585721001029014587402343750e-01;
      b[6] =  1.118876366882113199130444058937428053468465805053710938e-01;
      a[0] =  1.000000000000000000000000000000000000000000000000000000e+00;
      a[1] = -4.726938565651158441482948546763509511947631835937500000e+00;
      a[2] =  9.076897983832765248735086061060428619384765625000000000e+00;
      a[3] = -9.014855113464800950850985827855765819549560546875000000e+00;
      a[4] =  4.852772261031594425162438710685819387435913085937500000e+00;
      a[5] = -1.333877820398965186043938047077972441911697387695312500e+00;
      a[6] =  1.460012549591642450064199465487035922706127166748046875e-01;
      break;

    case 96000:
      b[0] =  9.951898975972744976203898659150581806898117065429687500e-02;
      b[1] = -1.990379795194548995240779731830116361379623413085937500e-01;
      b[2] = -9.951898975972744976203898659150581806898117065429687500e-02;
      b[3] =  3.980759590389097990481559463660232722759246826171875000e-01;
      b[4] = -9.951898975972744976203898659150581806898117065429687500e-02;
      b[5] = -1.990379795194548995240779731830116361379623413085937500e-01;
      b[6] =  9.951898975972744976203898659150581806898117065429687500e-02;
      a[0] =  1.000000000000000000000000000000000000000000000000000000e+00;
      a[1] = -4.802203044225376693532325589330866932868957519531250000e+00;
      a[2] =  9.401807218627226347962277941405773162841796875000000000e+00;
      a[3] = -9.566143943569164420637207513209432363510131835937500000e+00;
      a[4] =  5.309775930392619081032989925006404519081115722656250000e+00;
      a[5] = -1.517333360452622237346531619550660252571105957031250000e+00;
      a[6] =  1.740971994228911745583587844521389342844486236572265625e-01;
      break;

    case 192000:
      b[0] =  3.433213424548713782469278044118254911154508590698242188e-02;
      b[1] = -6.866426849097426177159775306790834292769432067871093750e-02;
      b[2] = -3.433213424548714476358668434841092675924301147460937500e-02;
      b[3] =  1.373285369819485235431955061358166858553886413574218750e-01;
      b[4] = -3.433213424548714476358668434841092675924301147460937500e-02;
      b[5] = -6.866426849097426177159775306790834292769432067871093750e-02;
      b[6] =  3.433213424548713782469278044118254911154508590698242188e-02;
      a[0] =  1.000000000000000000000000000000000000000000000000000000e+00;
      a[1] = -5.305923689674640009172890131594613194465637207031250000e+00;
      a[2] =  1.165952437466175695135461864992976188659667968750000000e+01;
      a[3] = -1.357560092700591525272102444432675838470458984375000000e+01;
      a[4] =  8.828906932824192921316353022120893001556396484375000000e+00;
      a[5] = -3.039490120988216581565666274400427937507629394531250000e+00;
      a[6] =  4.325834301870381537469256727490574121475219726562500000e-01;
      break;

    default:
      AUBIO_ERROR ("sampling rate of A-weighting filter is %d, should be one of\
 8000, 11025, 16000, 22050, 24000, 32000, 44100, 48000, 88200, 96000, 192000.\n", samplerate);
      return 1;

  }

  return 0;
}

aubio_filter_t *
new_aubio_filter_a_weighting (uint_t samplerate)
{
  aubio_filter_t *f = new_aubio_filter (7);
  aubio_filter_set_a_weighting (f, samplerate);
  return f;
}
