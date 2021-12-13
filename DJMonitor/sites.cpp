#include "sites.hpp"

#include "globals.hpp"
#include "functions.hpp"

#include <fstream>
#include <string>
#include <filesystem>

using namespace std;

unordered_map<string, string> *site_map;

void build_default_site_map();

vector<wxString> default_site_list() {
    if (!site_map || site_map->empty()) {
        build_default_site_map();
    }
    vector<wxString>outp;
    outp.reserve(site_map->size());
    for (const pair<string, string> &it : *site_map) {
        outp.push_back(it.first);
    }
    sort(outp.begin(), outp.end());
    return outp;
}

vector<wxString> get_site_list() {
    fstream site_list(settingdir + "\\sites.txt");
    if (!site_list) return default_site_list();
    string vers;
    getline(site_list, vers);
    // Built-in site map version defined here
    if (cmp_version("1.0", vers)) {
        // If built-in version >= version in file
        site_list.close();
        filesystem::remove(settingdir + "\\sites.txt");
        return default_site_list();
    }
    vector<wxString> outp;
    constexpr short expected_sites = 726;
    outp.reserve(expected_sites);
    site_map->reserve(expected_sites);
    string site, tz;
    bool badfile = false;
    while (!site_list.eof()) {
        if (getline(site_list, site, ':')) {
            if (getline(site_list, tz)) {
                outp.push_back(site);
                site_map->at(site) = tz;
            } else {
                badfile = true;
            }
        } else {
            badfile = true;
        }
    }
    site_list.close();
    if (badfile) {
        site_map->clear();
        return default_site_list();
    }
    return outp;
}

void build_default_site_map() {
    /*META-COMMENT:
    * The list of sites has been replaced with a list of airports and their (presumed) time zones.
    * As this isn't the intended purpose the accuracy of the below time zones has not been verified.
    */

    // Built-in site map defined here
    // Yes it's big.  It may be unnecessary; might be better to distribute with a default file
    // But I'd rather completely standalone
    site_map = new unordered_map<string, string> {
        {"BQN", "America/Puerto Rico"}, {"CPX", "America/Puerto Rico"}, {"MAZ", "America/Puerto Rico"},
        {"PSE", "America/Puerto Rico"}, {"SIG", "America/Puerto Rico"}, {"SJU", "America/Puerto Rico"},
        {"STT", "America/Puerto Rico"}, {"STX", "America/Puerto Rico"}, {"VQS", "America/Puerto Rico"},
        {"X95", "America/Puerto Rico"},

        {"GRO", "Pacific/Guam"}, {"GSN", "Pacific/Guam"}, {"GUM", "Pacific/Guam"},
        {"TNI", "Pacific/Guam"},

        {"0AK", "US/Alaska"}, {"16A", "US/Alaska"}, {"2A3", "US/Alaska"},
        {"2A9", "US/Alaska"}, {"3A5", "US/Alaska"}, {"6R7", "US/Alaska"},
        {"A61", "US/Alaska"}, {"A85", "US/Alaska"}, {"ADK", "US/Alaska"},
        {"ADQ", "US/Alaska"}, {"AFM", "US/Alaska"}, {"AGN", "US/Alaska"},
        {"AKN", "US/Alaska"}, {"AKP", "US/Alaska"}, {"AKW", "US/Alaska"},
        {"ANC", "US/Alaska"}, {"ANI", "US/Alaska"}, {"AQH", "US/Alaska"},
        {"AQT", "US/Alaska"}, {"ATK", "US/Alaska"}, {"AUK", "US/Alaska"},
        {"AWI", "US/Alaska"}, {"BET", "US/Alaska"}, {"BIG", "US/Alaska"},
        {"BRW", "US/Alaska"}, {"BTI", "US/Alaska"}, {"BVK", "US/Alaska"},
        {"CDB", "US/Alaska"}, {"CDV", "US/Alaska"}, {"CFK", "US/Alaska"},
        {"CGA", "US/Alaska"}, {"D76", "US/Alaska"}, {"DLG", "US/Alaska"},
        {"DUT", "US/Alaska"}, {"DUY", "US/Alaska"}, {"EEK", "US/Alaska"},
        {"ELI", "US/Alaska"}, {"ENA", "US/Alaska"}, {"ENM", "US/Alaska"},
        {"EWU", "US/Alaska"}, {"FAI", "US/Alaska"}, {"FYU", "US/Alaska"},
        {"GAL", "US/Alaska"}, {"GAM", "US/Alaska"}, {"GBH", "US/Alaska"},
        {"GGV", "US/Alaska"}, {"GKN", "US/Alaska"}, {"GST", "US/Alaska"},
        {"HLA", "US/Alaska"}, {"HNH", "US/Alaska"}, {"HNS", "US/Alaska"},
        {"HOM", "US/Alaska"}, {"HPB", "US/Alaska"}, {"HSL", "US/Alaska"},
        {"HYL", "US/Alaska"}, {"IAN", "US/Alaska"}, {"IIK", "US/Alaska"},
        {"ILI", "US/Alaska"}, {"JNU", "US/Alaska"}, {"KAE", "US/Alaska"},
        {"KAL", "US/Alaska"}, {"KDK", "US/Alaska"}, {"KEB", "US/Alaska"},
        {"KKA", "US/Alaska"}, {"KLG", "US/Alaska"}, {"KSM", "US/Alaska"},
        {"KTB", "US/Alaska"}, {"KTN", "US/Alaska"}, {"KVC", "US/Alaska"},
        {"KVL", "US/Alaska"}, {"KWT", "US/Alaska"}, {"LHD", "US/Alaska"},
        {"MBA", "US/Alaska"}, {"MCG", "US/Alaska"}, {"MDM", "US/Alaska"},
        {"MLL", "US/Alaska"}, {"MOU", "US/Alaska"}, {"MRI", "US/Alaska"},
        {"MTM", "US/Alaska"}, {"N93", "US/Alaska"}, {"NUL", "US/Alaska"},
        {"OME", "US/Alaska"}, {"OOK", "US/Alaska"}, {"ORI", "US/Alaska"},
        {"ORV", "US/Alaska"}, {"OTZ", "US/Alaska"}, {"PGM", "US/Alaska"},
        {"PHO", "US/Alaska"}, {"PPC", "US/Alaska"}, {"PSG", "US/Alaska"},
        {"PTH", "US/Alaska"}, {"RSH", "US/Alaska"}, {"SCC", "US/Alaska"},
        {"SCM", "US/Alaska"}, {"SDP", "US/Alaska"}, {"SGY", "US/Alaska"},
        {"SHG", "US/Alaska"}, {"SHH", "US/Alaska"}, {"SIT", "US/Alaska"},
        {"SNP", "US/Alaska"}, {"SOV", "US/Alaska"}, {"SVA", "US/Alaska"},
        {"T44", "US/Alaska"}, {"TAL", "US/Alaska"}, {"TLT", "US/Alaska"},
        {"TOG", "US/Alaska"}, {"UNK", "US/Alaska"}, {"VAK", "US/Alaska"},
        {"VDZ", "US/Alaska"}, {"WBB", "US/Alaska"}, {"WLK", "US/Alaska"},
        {"WNA", "US/Alaska"}, {"WRG", "US/Alaska"}, {"WTK", "US/Alaska"},
        {"YAK", "US/Alaska"}, {"Z09", "US/Alaska"},

        {"1G4", "US/Arizona"}, {"DVT", "US/Arizona"}, {"FHU", "US/Arizona"},
        {"FLG", "US/Arizona"}, {"GCN", "US/Arizona"}, {"GYR", "US/Arizona"},
        {"HII", "US/Arizona"}, {"IFP", "US/Arizona"}, {"IGM", "US/Arizona"},
        {"IWA", "US/Arizona"}, {"MZJ", "US/Arizona"}, {"PGA", "US/Arizona"},
        {"PHX", "US/Arizona"}, {"PRC", "US/Arizona"}, {"SOW", "US/Arizona"},
        {"TUS", "US/Arizona"}, {"YUM", "US/Arizona"},

        {"ACB", "US/Central"}, {"AEX", "US/Central"}, {"AIA", "US/Central"},
        {"AIZ", "US/Central"}, {"ALN", "US/Central"}, {"ALO", "US/Central"},
        {"ANB", "US/Central"}, {"APN", "US/Central"}, {"ASN", "US/Central"},
        {"ATW", "US/Central"}, {"AXN", "US/Central"}, {"AZO", "US/Central"},
        {"BDE", "US/Central"}, {"BEH", "US/Central"}, {"BFF", "US/Central"},
        {"BHM", "US/Central"}, {"BJI", "US/Central"}, {"BLV", "US/Central"},
        {"BMI", "US/Central"}, {"BNA", "US/Central"}, {"BPK", "US/Central"},
        {"BRD", "US/Central"}, {"BRL", "US/Central"}, {"BTL", "US/Central"},
        {"BTR", "US/Central"}, {"BWG", "US/Central"}, {"CGI", "US/Central"},
        {"CGX", "US/Central"}, {"CHA", "US/Central"}, {"CID", "US/Central"},
        {"CIU", "US/Central"}, {"CMI", "US/Central"}, {"CMX", "US/Central"},
        {"COU", "US/Central"}, {"CVX", "US/Central"}, {"CWA", "US/Central"},
        {"CWI", "US/Central"}, {"DBQ", "US/Central"}, {"DDC", "US/Central"},
        {"DEC", "US/Central"}, {"DET", "US/Central"}, {"DHN", "US/Central"},
        {"DLH", "US/Central"}, {"DNV", "US/Central"}, {"DSM", "US/Central"},
        {"DTW", "US/Central"}, {"EAR", "US/Central"}, {"EAU", "US/Central"},
        {"ELO", "US/Central"}, {"ENW", "US/Central"}, {"ESC", "US/Central"},
        {"ESF", "US/Central"}, {"FNT", "US/Central"}, {"FOD", "US/Central"},
        {"FOE", "US/Central"}, {"FRM", "US/Central"}, {"FSM", "US/Central"},
        {"FYV", "US/Central"}, {"GBD", "US/Central"}, {"GCK", "US/Central"},
        {"GLD", "US/Central"}, {"GLH", "US/Central"}, {"GLR", "US/Central"},
        {"GPT", "US/Central"}, {"GPZ", "US/Central"}, {"GRB", "US/Central"},
        {"GRI", "US/Central"}, {"GRR", "US/Central"}, {"GTR", "US/Central"},
        {"HIB", "US/Central"}, {"HKS", "US/Central"}, {"HOT", "US/Central"},
        {"HRO", "US/Central"}, {"HSV", "US/Central"}, {"HUT", "US/Central"},
        {"HYS", "US/Central"}, {"ICT", "US/Central"}, {"ILL", "US/Central"},
        {"IMT", "US/Central"}, {"INL", "US/Central"}, {"IRK", "US/Central"},
        {"IWD", "US/Central"}, {"IXD", "US/Central"}, {"JAN", "US/Central"},
        {"JBR", "US/Central"}, {"JEF", "US/Central"}, {"JLN", "US/Central"},
        {"JVL", "US/Central"}, {"JXN", "US/Central"}, {"LAN", "US/Central"},
        {"LAW", "US/Central"}, {"LBF", "US/Central"}, {"LBL", "US/Central"},
        {"LCH", "US/Central"}, {"LEX", "US/Central"}, {"LFT", "US/Central"},
        {"LIT", "US/Central"}, {"LNK", "US/Central"}, {"LSE", "US/Central"},
        {"MBL", "US/Central"}, {"MBS", "US/Central"}, {"MCI", "US/Central"},
        {"MCK", "US/Central"}, {"MCW", "US/Central"}, {"MDH", "US/Central"},
        {"MDW", "US/Central"}, {"MEI", "US/Central"}, {"MEM", "US/Central"},
        {"MGM", "US/Central"}, {"MHK", "US/Central"}, {"MKC", "US/Central"},
        {"MKE", "US/Central"}, {"MKG", "US/Central"}, {"MKL", "US/Central"},
        {"MKT", "US/Central"}, {"MLI", "US/Central"}, {"MLU", "US/Central"},
        {"MNM", "US/Central"}, {"MOB", "US/Central"}, {"MQY", "US/Central"},
        {"MSL", "US/Central"}, {"MSN", "US/Central"}, {"MSP", "US/Central"},
        {"MSY", "US/Central"}, {"MTO", "US/Central"}, {"MVN", "US/Central"},
        {"MWA", "US/Central"}, {"NEW", "US/Central"}, {"NQA", "US/Central"},
        {"OFK", "US/Central"}, {"OKC", "US/Central"}, {"OMA", "US/Central"},
        {"ORD", "US/Central"}, {"OSH", "US/Central"}, {"OTG", "US/Central"},
        {"OTM", "US/Central"}, {"OWB", "US/Central"}, {"PAH", "US/Central"},
        {"PIA", "US/Central"}, {"PIB", "US/Central"}, {"PLK", "US/Central"},
        {"PLN", "US/Central"}, {"PNC", "US/Central"}, {"PQL", "US/Central"},
        {"PTK", "US/Central"}, {"RFD", "US/Central"}, {"RHI", "US/Central"},
        {"RST", "US/Central"}, {"SAW", "US/Central"}, {"SDF", "US/Central"},
        {"SGF", "US/Central"}, {"SHV", "US/Central"}, {"SLN", "US/Central"},
        {"SPI", "US/Central"}, {"SQI", "US/Central"}, {"STC", "US/Central"},
        {"STJ", "US/Central"}, {"STL", "US/Central"}, {"STP", "US/Central"},
        {"SUS", "US/Central"}, {"SUX", "US/Central"}, {"SWO", "US/Central"},
        {"TBN", "US/Central"}, {"TCL", "US/Central"}, {"TRI", "US/Central"},
        {"TUL", "US/Central"}, {"TUP", "US/Central"}, {"TVC", "US/Central"},
        {"TVF", "US/Central"}, {"TVR", "US/Central"}, {"TXK", "US/Central"},
        {"TYS", "US/Central"}, {"UIN", "US/Central"}, {"UOX", "US/Central"},
        {"WDG", "US/Central"}, {"XNA", "US/Central"},

        {"3T7", "US/Eastern"}, {"3W2", "US/Eastern"}, {"ABE", "US/Eastern"},
        {"ABI", "US/Eastern"}, {"ABR", "US/Eastern"}, {"ABY", "US/Eastern"},
        {"ACK", "US/Eastern"}, {"ACT", "US/Eastern"}, {"ACY", "US/Eastern"},
        {"AGC", "US/Eastern"}, {"AGS", "US/Eastern"}, {"AHN", "US/Eastern"},
        {"AID", "US/Eastern"}, {"AIY", "US/Eastern"}, {"ALB", "US/Eastern"},
        {"AMA", "US/Eastern"}, {"AND", "US/Eastern"}, {"AOO", "US/Eastern"},
        {"APF", "US/Eastern"}, {"ART", "US/Eastern"}, {"ATL", "US/Eastern"},
        {"ATY", "US/Eastern"}, {"AUG", "US/Eastern"}, {"AUS", "US/Eastern"},
        {"AVL", "US/Eastern"}, {"AVP", "US/Eastern"}, {"BAF", "US/Eastern"},
        {"BAK", "US/Eastern"}, {"BDL", "US/Eastern"}, {"BDR", "US/Eastern"},
        {"BED", "US/Eastern"}, {"BFD", "US/Eastern"}, {"BGM", "US/Eastern"},
        {"BGR", "US/Eastern"}, {"BHB", "US/Eastern"}, {"BID", "US/Eastern"},
        {"BIS", "US/Eastern"}, {"BKL", "US/Eastern"}, {"BKW", "US/Eastern"},
        {"BKX", "US/Eastern"}, {"BLM", "US/Eastern"}, {"BMG", "US/Eastern"},
        {"BOS", "US/Eastern"}, {"BPT", "US/Eastern"}, {"BQK", "US/Eastern"},
        {"BRO", "US/Eastern"}, {"BTV", "US/Eastern"}, {"BUF", "US/Eastern"},
        {"BWI", "US/Eastern"}, {"CAE", "US/Eastern"}, {"CAK", "US/Eastern"},
        {"CDW", "US/Eastern"}, {"CEF", "US/Eastern"}, {"CGF", "US/Eastern"},
        {"CHO", "US/Eastern"}, {"CHS", "US/Eastern"}, {"CKB", "US/Eastern"},
        {"CLE", "US/Eastern"}, {"CLL", "US/Eastern"}, {"CLT", "US/Eastern"},
        {"CMH", "US/Eastern"}, {"CRP", "US/Eastern"}, {"CRW", "US/Eastern"},
        {"CSG", "US/Eastern"}, {"CVG", "US/Eastern"}, {"DAB", "US/Eastern"},
        {"DAL", "US/Eastern"}, {"DAN", "US/Eastern"}, {"DAY", "US/Eastern"},
        {"DCA", "US/Eastern"}, {"DDH", "US/Eastern"}, {"DFW", "US/Eastern"},
        {"DIK", "US/Eastern"}, {"DRT", "US/Eastern"}, {"DUJ", "US/Eastern"},
        {"DVL", "US/Eastern"}, {"DXR", "US/Eastern"}, {"EEN", "US/Eastern"},
        {"EFD", "US/Eastern"}, {"EFK", "US/Eastern"}, {"EKM", "US/Eastern"},
        {"ELM", "US/Eastern"}, {"ELP", "US/Eastern"}, {"ERI", "US/Eastern"},
        {"EVV", "US/Eastern"}, {"EWB", "US/Eastern"}, {"EWN", "US/Eastern"},
        {"EWR", "US/Eastern"}, {"EYW", "US/Eastern"}, {"FAR", "US/Eastern"},
        {"FAY", "US/Eastern"}, {"FKL", "US/Eastern"}, {"FLL", "US/Eastern"},
        {"FLO", "US/Eastern"}, {"FRG", "US/Eastern"}, {"FSD", "US/Eastern"},
        {"FTW", "US/Eastern"}, {"FWA", "US/Eastern"}, {"GED", "US/Eastern"},
        {"GFK", "US/Eastern"}, {"GFL", "US/Eastern"}, {"GGG", "US/Eastern"},
        {"GLS", "US/Eastern"}, {"GNV", "US/Eastern"}, {"GON", "US/Eastern"},
        {"GRK", "US/Eastern"}, {"GSO", "US/Eastern"}, {"GSP", "US/Eastern"},
        {"GYH", "US/Eastern"}, {"GYY", "US/Eastern"}, {"HFD", "US/Eastern"},
        {"HGR", "US/Eastern"}, {"HKY", "US/Eastern"}, {"HON", "US/Eastern"},
        {"HOU", "US/Eastern"}, {"HPN", "US/Eastern"}, {"HRL", "US/Eastern"},
        {"HTS", "US/Eastern"}, {"HUF", "US/Eastern"}, {"HVN", "US/Eastern"},
        {"HXD", "US/Eastern"}, {"HYA", "US/Eastern"}, {"IAD", "US/Eastern"},
        {"IAH", "US/Eastern"}, {"ILE", "US/Eastern"}, {"ILG", "US/Eastern"},
        {"ILM", "US/Eastern"}, {"IND", "US/Eastern"}, {"INT", "US/Eastern"},
        {"IPT", "US/Eastern"}, {"ISN", "US/Eastern"}, {"ISO", "US/Eastern"},
        {"ISP", "US/Eastern"}, {"ITH", "US/Eastern"}, {"JAX", "US/Eastern"},
        {"JFK", "US/Eastern"}, {"JHW", "US/Eastern"}, {"JMS", "US/Eastern"},
        {"JRB", "US/Eastern"}, {"JST", "US/Eastern"}, {"LAF", "US/Eastern"},
        {"LAL", "US/Eastern"}, {"LBB", "US/Eastern"}, {"LBE", "US/Eastern"},
        {"LBX", "US/Eastern"}, {"LCK", "US/Eastern"}, {"LEB", "US/Eastern"},
        {"LGA", "US/Eastern"}, {"LNS", "US/Eastern"}, {"LPR", "US/Eastern"},
        {"LRD", "US/Eastern"}, {"LUK", "US/Eastern"}, {"LWB", "US/Eastern"},
        {"LYH", "US/Eastern"}, {"MAF", "US/Eastern"}, {"MCN", "US/Eastern"},
        {"MCO", "US/Eastern"}, {"MDT", "US/Eastern"}, {"MFD", "US/Eastern"},
        {"MFE", "US/Eastern"}, {"MGW", "US/Eastern"}, {"MHE", "US/Eastern"},
        {"MHT", "US/Eastern"}, {"MIA", "US/Eastern"}, {"MIE", "US/Eastern"},
        {"MIV", "US/Eastern"}, {"MJX", "US/Eastern"}, {"MLB", "US/Eastern"},
        {"MMU", "US/Eastern"}, {"MOT", "US/Eastern"}, {"MPV", "US/Eastern"},
        {"MQI", "US/Eastern"}, {"MQJ", "US/Eastern"}, {"MSS", "US/Eastern"},
        {"MSV", "US/Eastern"}, {"MTH", "US/Eastern"}, {"MVL", "US/Eastern"},
        {"MVY", "US/Eastern"}, {"MYR", "US/Eastern"}, {"OAJ", "US/Eastern"},
        {"OCF", "US/Eastern"}, {"OGS", "US/Eastern"}, {"OQU", "US/Eastern"},
        {"ORF", "US/Eastern"}, {"ORH", "US/Eastern"}, {"OSU", "US/Eastern"},
        {"OXC", "US/Eastern"}, {"PBI", "US/Eastern"}, {"PCW", "US/Eastern"},
        {"PFN", "US/Eastern"}, {"PGD", "US/Eastern"}, {"PGV", "US/Eastern"},
        {"PHF", "US/Eastern"}, {"PHL", "US/Eastern"}, {"PIE", "US/Eastern"},
        {"PIR", "US/Eastern"}, {"PIT", "US/Eastern"}, {"PKB", "US/Eastern"},
        {"PLB", "US/Eastern"}, {"PNS", "US/Eastern"}, {"POU", "US/Eastern"},
        {"PQI", "US/Eastern"}, {"PSM", "US/Eastern"}, {"PVC", "US/Eastern"},
        {"PVD", "US/Eastern"}, {"PWM", "US/Eastern"}, {"RAP", "US/Eastern"},
        {"RDG", "US/Eastern"}, {"RDU", "US/Eastern"}, {"RIC", "US/Eastern"},
        {"RKD", "US/Eastern"}, {"RME", "US/Eastern"}, {"RMG", "US/Eastern"},
        {"ROA", "US/Eastern"}, {"ROC", "US/Eastern"}, {"RSW", "US/Eastern"},
        {"RUT", "US/Eastern"}, {"RWI", "US/Eastern"}, {"SAT", "US/Eastern"},
        {"SAV", "US/Eastern"}, {"SBN", "US/Eastern"}, {"SBY", "US/Eastern"},
        {"SFB", "US/Eastern"}, {"SFZ", "US/Eastern"}, {"SGH", "US/Eastern"},
        {"SGJ", "US/Eastern"}, {"SHD", "US/Eastern"}, {"SJT", "US/Eastern"},
        {"SLK", "US/Eastern"}, {"SOP", "US/Eastern"}, {"SPS", "US/Eastern"},
        {"SRQ", "US/Eastern"}, {"SWF", "US/Eastern"}, {"SYR", "US/Eastern"},
        {"TEB", "US/Eastern"}, {"TIX", "US/Eastern"}, {"TLH", "US/Eastern"},
        {"TOL", "US/Eastern"}, {"TPA", "US/Eastern"}, {"TPL", "US/Eastern"},
        {"TTN", "US/Eastern"}, {"TYR", "US/Eastern"}, {"UCA", "US/Eastern"},
        {"UNV", "US/Eastern"}, {"UUU", "US/Eastern"}, {"VCT", "US/Eastern"},
        {"VLD", "US/Eastern"}, {"VPS", "US/Eastern"}, {"VPZ", "US/Eastern"},
        {"VQQ", "US/Eastern"}, {"VRB", "US/Eastern"}, {"WST", "US/Eastern"},
        {"WWD", "US/Eastern"}, {"X44", "US/Eastern"}, {"YKN", "US/Eastern"},
        {"YNG", "US/Eastern"},

        {"HNL", "US/Hawaii"}, {"HNM", "US/Hawaii"}, {"ITO", "US/Hawaii"},
        {"KOA", "US/Hawaii"}, {"LIH", "US/Hawaii"}, {"LNY", "US/Hawaii"},
        {"MDY", "US/Hawaii"}, {"MKK", "US/Hawaii"}, {"MUE", "US/Hawaii"},
        {"OGG", "US/Hawaii"},

        {"ABQ", "US/Mountain"}, {"ALM", "US/Mountain"}, {"ALS", "US/Mountain"},
        {"ASE", "US/Mountain"}, {"BCE", "US/Mountain"}, {"BIL", "US/Mountain"},
        {"BTM", "US/Mountain"}, {"BZN", "US/Mountain"}, {"CDC", "US/Mountain"},
        {"CEZ", "US/Mountain"}, {"CNM", "US/Mountain"}, {"CNY", "US/Mountain"},
        {"COD", "US/Mountain"}, {"COS", "US/Mountain"}, {"CPR", "US/Mountain"},
        {"CYS", "US/Mountain"}, {"DEN", "US/Mountain"}, {"DRO", "US/Mountain"},
        {"EGE", "US/Mountain"}, {"FMN", "US/Mountain"}, {"FNL", "US/Mountain"},
        {"GCC", "US/Mountain"}, {"GGW", "US/Mountain"}, {"GJT", "US/Mountain"},
        {"GPI", "US/Mountain"}, {"GTF", "US/Mountain"}, {"GUC", "US/Mountain"},
        {"GUP", "US/Mountain"}, {"HDN", "US/Mountain"}, {"HLN", "US/Mountain"},
        {"HOB", "US/Mountain"}, {"JAC", "US/Mountain"}, {"LAA", "US/Mountain"},
        {"LAR", "US/Mountain"}, {"LRU", "US/Mountain"}, {"MSO", "US/Mountain"},
        {"MTJ", "US/Mountain"}, {"OGD", "US/Mountain"}, {"PUB", "US/Mountain"},
        {"PVU", "US/Mountain"}, {"RIW", "US/Mountain"}, {"RKS", "US/Mountain"},
        {"ROW", "US/Mountain"}, {"SAF", "US/Mountain"}, {"SDY", "US/Mountain"},
        {"SGU", "US/Mountain"}, {"SHR", "US/Mountain"}, {"SKX", "US/Mountain"},
        {"SLC", "US/Mountain"}, {"SRR", "US/Mountain"}, {"SVC", "US/Mountain"},
        {"TEX", "US/Mountain"}, {"WRL", "US/Mountain"}, {"WYS", "US/Mountain"},

        {"74S", "US/Pacific"}, {"ACV", "US/Pacific"}, {"ALW", "US/Pacific"},
        {"AST", "US/Pacific"}, {"BFI", "US/Pacific"}, {"BFL", "US/Pacific"},
        {"BLI", "US/Pacific"}, {"BOI", "US/Pacific"}, {"BUR", "US/Pacific"},
        {"CCR", "US/Pacific"}, {"CEC", "US/Pacific"}, {"CIC", "US/Pacific"},
        {"CLM", "US/Pacific"}, {"COE", "US/Pacific"}, {"CRQ", "US/Pacific"},
        {"CVO", "US/Pacific"}, {"EAT", "US/Pacific"}, {"EKO", "US/Pacific"},
        {"ELY", "US/Pacific"}, {"EUG", "US/Pacific"}, {"FAT", "US/Pacific"},
        {"FHR", "US/Pacific"}, {"GEG", "US/Pacific"}, {"HND", "US/Pacific"},
        {"IDA", "US/Pacific"}, {"IPL", "US/Pacific"}, {"IYK", "US/Pacific"},
        {"LAS", "US/Pacific"}, {"LAX", "US/Pacific"}, {"LGB", "US/Pacific"},
        {"LMT", "US/Pacific"}, {"LWS", "US/Pacific"}, {"MCC", "US/Pacific"},
        {"MCE", "US/Pacific"}, {"MFR", "US/Pacific"}, {"MMH", "US/Pacific"},
        {"MMV", "US/Pacific"}, {"MOD", "US/Pacific"}, {"MRY", "US/Pacific"},
        {"MWH", "US/Pacific"}, {"OAK", "US/Pacific"}, {"OLM", "US/Pacific"},
        {"ONP", "US/Pacific"}, {"ONT", "US/Pacific"}, {"ORS", "US/Pacific"},
        {"OTH", "US/Pacific"}, {"OXR", "US/Pacific"}, {"PAE", "US/Pacific"},
        {"PDT", "US/Pacific"}, {"PDX", "US/Pacific"}, {"PIH", "US/Pacific"},
        {"PMD", "US/Pacific"}, {"PRB", "US/Pacific"}, {"PSC", "US/Pacific"},
        {"PSP", "US/Pacific"}, {"PUW", "US/Pacific"}, {"PWT", "US/Pacific"},
        {"RDD", "US/Pacific"}, {"RDM", "US/Pacific"}, {"RNO", "US/Pacific"},
        {"SAN", "US/Pacific"}, {"SBA", "US/Pacific"}, {"SBD", "US/Pacific"},
        {"SBP", "US/Pacific"}, {"SCK", "US/Pacific"}, {"SEA", "US/Pacific"},
        {"SFO", "US/Pacific"}, {"SJC", "US/Pacific"}, {"SMF", "US/Pacific"},
        {"SMX", "US/Pacific"}, {"SNA", "US/Pacific"}, {"STS", "US/Pacific"},
        {"SUN", "US/Pacific"}, {"TVL", "US/Pacific"}, {"TWF", "US/Pacific"},
        {"VCV", "US/Pacific"}, {"VGT", "US/Pacific"}, {"VIS", "US/Pacific"},
        {"YKM", "US/Pacific"},

        {"FAQ", "US/Samoa"}, {"PPG", "US/Samoa"}, {"Z08", "US/Samoa"},
    };
}
