#include "../source/database.h"
#include "../utils/clibs/analysis_utils.h"
//
void check_overvoltage_noise(std::vector<std::string> steps_to_investigate)
{
    //  Suppress verbose
    gErrorIgnoreLevel = 6001;
    //  Load database
    database::basedir = get_environment_variable("SIPM4EIC_DATA");
    database::read_database(database::basedir + "/database.txt");
    //  Data structures
    std::map<TString, TH1F *> _TH1F;
    std::map<TString, TH2F *> _TH2F;
    std::map<TString, TGraphErrors *> _TGraphErrors;
    //  --- Results histograms
    for (auto [current_sensor_short, current_channel] : database::sensor_short_to_channels)
    {
        for (auto current_status : steps_to_investigate)
        {
            //  Dark counts
            auto key_name = TString(current_sensor_short) + "_DCR_" + TString(current_status);
            _TH1F[key_name] = new TH1F(key_name, ";Dark counts (Hz);Entries", 100, 1.e1, 1.e7);
            set_log_binning(_TH1F[key_name]);
            _TH1F[key_name]->SetLineColor(database::sensor_short_to_color[current_sensor_short]);
            key_name = TString(current_sensor_short) + "_DCR_DiffNEW_" + TString(current_status);
            _TH1F[key_name] = new TH1F(key_name, ";Dark counts (Hz);Entries", 100, 1.e1, 1.e7);
            set_log_binning(_TH1F[key_name]);
            _TH1F[key_name]->SetLineColor(database::sensor_short_to_color[current_sensor_short]);
            //  Dark currents
            key_name = TString(current_sensor_short) + "_IV_" + TString(current_status);
            _TH1F[key_name] = new TH1F(key_name, ";Dark current (A);Entries", 100, 1.e-10, 1.e-4);
            set_log_binning(_TH1F[key_name]);
            _TH1F[key_name]->SetLineColor(database::sensor_short_to_color[current_sensor_short]);
            key_name = TString(current_sensor_short) + "_IV_DiffNEW_" + TString(current_status);
            _TH1F[key_name] = new TH1F(key_name, ";Dark current (A);Entries", 100, 1.e-10, 1.e-4);
            set_log_binning(_TH1F[key_name]);
            _TH1F[key_name]->SetLineColor(database::sensor_short_to_color[current_sensor_short]);
            //  Gain
            key_name = TString(current_sensor_short) + "_Gain_" + TString(current_status);
            _TH1F[key_name] = new TH1F(key_name, ";Sensor gain;Entries", 100, 1.e5, 1.e8);
            set_log_binning(_TH1F[key_name]);
            _TH1F[key_name]->SetLineColor(database::sensor_short_to_color[current_sensor_short]);
            //
            //
            //  Dark counts
            key_name = TString(current_sensor_short) + "_DCR_" + TString(current_status);
            _TH2F[key_name] = new TH2F(key_name, ";Dark counts (Hz);Entries", 100, 1.e1, 1.e7, 100, 0., 9.);
            set_log_binning(_TH2F[key_name]);
            _TH2F[key_name]->SetLineColor(database::sensor_short_to_color[current_sensor_short]);
            key_name = TString(current_sensor_short) + "_DCR_DiffNEW_" + TString(current_status);
            _TH2F[key_name] = new TH2F(key_name, ";Dark counts (Hz);Entries", 100, 1.e1, 1.e7, 100, 0., 9.);
            set_log_binning(_TH2F[key_name]);
            _TH2F[key_name]->SetLineColor(database::sensor_short_to_color[current_sensor_short]);
            //  Dark currents
            key_name = TString(current_sensor_short) + "_IV_" + TString(current_status);
            _TH2F[key_name] = new TH2F(key_name, ";Dark current (A);Entries", 100, 1.e-10, 1.e-4, 100, 0., 9.);
            set_log_binning(_TH2F[key_name]);
            _TH2F[key_name]->SetLineColor(database::sensor_short_to_color[current_sensor_short]);
            key_name = TString(current_sensor_short) + "_IV_DiffNEW_" + TString(current_status);
            _TH2F[key_name] = new TH2F(key_name, ";Dark current (A);Entries", 100, 1.e-10, 1.e-4, 100, 0., 9.);
            set_log_binning(_TH2F[key_name]);
            _TH2F[key_name]->SetLineColor(database::sensor_short_to_color[current_sensor_short]);
            //  Gain
            key_name = TString(current_sensor_short) + "_Gain_" + TString(current_status);
            _TH2F[key_name] = new TH2F(key_name, ";Sensor gain;Entries", 100, 1.e5, 1.e8, 100, 0., 9);
            set_log_binning(_TH2F[key_name]);
            _TH2F[key_name]->SetLineColor(database::sensor_short_to_color[current_sensor_short]);
        }
    }
    //  Calculate all Vbd
    database::calculate_breakdown_voltages();
    //  Start loop over boards
    for (auto current_board : database::all_boards)
    {
        if ((database::board_to_purpose[current_board].find("onl") != std::string::npos))
            continue;
        //  Start loop over sensors / channels
        for (auto [current_sensor_short, current_channels] : database::sensor_short_to_channels)
        {
            for (auto current_channel : current_channels)
            {
                //  Get reference Vbd value ("TIFPA-IRR1")
                auto current_Vbd = database::get_breakdown_voltage(current_board, current_channel, "TIFPA-IRR1");
                //  Get reference Current / DCR value ("NEW")
                auto current_iv_NEW = database::get_iv_scan(current_board, current_channel, "NEW");
                auto current_dcr_NEW = database::get_dcr_vbias_scan(current_board, current_channel, "NEW");
                //  Calculate VoV@3V Current / DCR value ("NEW")
                auto current_A_VoV_NEW = eval_with_errors(shift_with_vbd<false, -1>(current_iv_NEW, get<0>(current_Vbd)), 3.);
                auto current_DCR_VoV_NEW = eval_with_errors(shift_with_vbd<false, -1>(current_dcr_NEW, get<0>(current_Vbd)), 3.);
                for (auto current_status : steps_to_investigate)
                {
                    //   Get under test Current / DCR / Gain
                    auto current_iv = database::get_iv_scan(current_board, current_channel, current_status);
                    if (!current_iv)
                        continue;
                    auto current_dcr = database::get_dcr_vbias_scan(current_board, current_channel, current_status);
                    if (!current_dcr)
                        continue;
                    auto current_gain = database::get_gain(current_board, current_channel, current_status);
                    if (!current_gain)
                        continue;
                    //  Measure VoV@3V
                    auto current_A_VoV = eval_with_errors(shift_with_vbd<false, -1>(current_iv, get<0>(current_Vbd)), 3.);
                    auto current_DCR_VoV = eval_with_errors(shift_with_vbd<false, -1>(current_dcr, get<0>(current_Vbd)), 3.);
                    auto current_Gain_VoV = eval_with_errors(shift_with_vbd<false, -1>(current_gain, get<0>(current_Vbd)), 3.);
                    //  Fill histograms
                    auto key_name = TString(current_sensor_short) + "_DCR_" + TString(current_status);
                    _TH1F[key_name]->Fill(get<0>(current_DCR_VoV));
                    key_name = TString(current_sensor_short) + "_DCR_DiffNEW_" + TString(current_status);
                    _TH1F[key_name]->Fill(get<0>(current_DCR_VoV) - get<0>(current_DCR_VoV_NEW));
                    //  Dark currents
                    key_name = TString(current_sensor_short) + "_IV_" + TString(current_status);
                    _TH1F[key_name]->Fill(get<0>(current_A_VoV));
                    key_name = TString(current_sensor_short) + "_IV_DiffNEW_" + TString(current_status);
                    _TH1F[key_name]->Fill(get<0>(current_A_VoV) - get<0>(current_A_VoV_NEW));
                    //  Gain
                    key_name = TString(current_sensor_short) + "_Gain_" + TString(current_status);
                    _TH1F[key_name]->Fill(get<0>(current_Gain_VoV));
                    for (int iVoV = 0; iVoV < 100; iVoV++)
                    {
                        auto current_x = iVoV * 9. / 100.;
                        current_A_VoV = eval_with_errors(shift_with_vbd<false, -1>(current_iv, get<0>(current_Vbd)), current_x);
                        current_DCR_VoV = eval_with_errors(shift_with_vbd<false, -1>(current_dcr, get<0>(current_Vbd)), current_x);
                        current_Gain_VoV = eval_with_errors(shift_with_vbd<false, -1>(current_gain, get<0>(current_Vbd)), current_x);
                        //
                        //  Fill histograms
                        auto key_name = TString(current_sensor_short) + "_DCR_" + TString(current_status);
                        _TH2F[key_name]->Fill(get<0>(current_DCR_VoV), current_x);
                        key_name = TString(current_sensor_short) + "_DCR_DiffNEW_" + TString(current_status);
                        _TH2F[key_name]->Fill(get<0>(current_DCR_VoV) - get<0>(current_DCR_VoV_NEW), current_x);
                        //  Dark currents
                        key_name = TString(current_sensor_short) + "_IV_" + TString(current_status);
                        _TH2F[key_name]->Fill(get<0>(current_A_VoV), current_x);
                        key_name = TString(current_sensor_short) + "_IV_DiffNEW_" + TString(current_status);
                        _TH2F[key_name]->Fill(get<0>(current_A_VoV) - get<0>(current_A_VoV_NEW), current_x);
                        //  Gain
                        key_name = TString(current_sensor_short) + "_Gain_" + TString(current_status);
                        _TH2F[key_name]->Fill(get<0>(current_Gain_VoV), current_x);
                    }
                }
            }
        }
    }
    //
    for (auto [current_sensor_short, current_channel] : database::sensor_short_to_channels)
    {
        //  Gain
        auto h_key_name = TString(current_sensor_short) + "_Gain_Spread";
        _TH1F[h_key_name] = new TH1F(h_key_name, ";;Spread of sensor gain (#sigma / #mu)", steps_to_investigate.size(), 0., steps_to_investigate.size());
        _TH1F[h_key_name]->SetLineColor(database::sensor_short_to_color[current_sensor_short]);
        auto iBin = 0;
        for (auto current_status : steps_to_investigate)
        {
            iBin++;
            auto key_name = TString(current_sensor_short) + "_Gain_" + TString(current_status);
            auto ratio = calc_ratio({_TH1F[key_name]->GetRMS(), _TH1F[key_name]->GetRMSError()}, {_TH1F[key_name]->GetMean(), _TH1F[key_name]->GetMeanError()});
            _TH1F[h_key_name]->SetBinContent(iBin, get<0>(ratio));
            _TH1F[h_key_name]->SetBinError(iBin, get<1>(ratio));
            _TH1F[h_key_name]->GetXaxis()->SetBinLabel(iBin, current_status.c_str());
        }
    }
    //
    //   Output file
    TFile *fOutputFile = new TFile("check_overvoltage_noise.root", "RECREATE");
    for (auto [name, histogram] : _TH1F)
        histogram->Write();
    for (auto [name, histogram] : _TH2F)
        histogram->Write();
    fOutputFile->Close();
}