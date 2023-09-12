#include "../source/database.h"
#include "../utils/clibs/analysis_utils.h"
//
void temperature_scan()
{
    std::map<std::string, double> status_to_quantity = database::status_to_temperature;
    std::map<TString, TH1F *> _TH1F;
    std::map<TString, TH1F *> _TH2F;
    std::map<TString, TGraphErrors *> _TGraphErrors;
    std::map<TString, TMultiGraph *> _TMultiGraph;
    gErrorIgnoreLevel = 6001;
    database::basedir = get_environment_variable("SIPM4EIC_DATA");
    database::read_database(database::basedir + "/database.txt");
    //  Calculate all Vbd
    //  database::calculate_breakdown_voltages();
    //
    //  Find the boards that underwent Temp scans
    std::vector<std::string> boards_for_scan;
    boards_for_scan.push_back("15");
    boards_for_scan.push_back("16");
    /*
    for (auto [current_board, status_list] : database::board_to_status_list)
    {
        for (auto current_status : status_list)
        {
            if ((current_status.find("TEMP") != std::string::npos))
            {
                boards_for_scan.push_back(current_board);
                break;
            }
        }
    }*/
    //
    //  Calculated reference VoV @20C
    std::map<TString, double> reference_Vbd;
    reference_Vbd["Hamamatsu S13361-3050"] = 51.4;
    reference_Vbd["Hamamatsu S13361-3075"] = 50.9;
    reference_Vbd["Hamamatsu S14161-3050"] = 38.8;
    //
    //  Define summary Multigraphs
    _TMultiGraph["_Vbd"] = new TMultiGraph();
    _TMultiGraph["_Vact"] = new TMultiGraph();
    _TMultiGraph["_VactVoV"] = new TMultiGraph();
    //
    auto iTer = -1;
    for (auto [current_sensor, current_channels] : database::sensor_to_channels)
    {
        iTer++;
        //  Vbd vs Temperature
        auto vbd_key_name = TString(current_sensor) + "_Vbd";
        _TGraphErrors[vbd_key_name] = new TGraphErrors();
        _TGraphErrors[vbd_key_name]->SetMarkerColor(database::sensor_to_color[current_sensor]);
        _TGraphErrors[vbd_key_name]->SetMarkerStyle(24 + iTer);
        _TGraphErrors[vbd_key_name]->SetMarkerSize(2);
        _TGraphErrors[vbd_key_name]->GetXaxis()->SetTitle("Temperature (K)");
        _TGraphErrors[vbd_key_name]->GetYaxis()->SetTitle("Voltage (V)");
        //  Vact in fixed voltage
        auto vact_key_name = TString(current_sensor) + "_Vact";
        _TGraphErrors[vact_key_name] = new TGraphErrors();
        _TGraphErrors[vact_key_name]->SetMarkerColor(database::sensor_to_color[current_sensor]);
        _TGraphErrors[vact_key_name]->SetMarkerStyle(24 + iTer);
        _TGraphErrors[vact_key_name]->SetMarkerSize(2);
        _TGraphErrors[vact_key_name]->GetXaxis()->SetTitle("1/kT (eV^{-1})");
        _TGraphErrors[vact_key_name]->GetYaxis()->SetTitle(Form("ln(I_{dark} @ %.1fV)", reference_Vbd[current_sensor] + 3));
        //  Vact in overvoltage
        auto vact_VoV_key_name = TString(current_sensor) + "_VactVoV";
        _TGraphErrors[vact_VoV_key_name] = new TGraphErrors();
        _TGraphErrors[vact_VoV_key_name]->SetMarkerColor(database::sensor_to_color[current_sensor]);
        _TGraphErrors[vact_VoV_key_name]->SetMarkerStyle(24 + iTer);
        _TGraphErrors[vact_VoV_key_name]->SetMarkerSize(2);
        _TGraphErrors[vact_VoV_key_name]->GetXaxis()->SetTitle("1/kT (eV^{-1})");
        _TGraphErrors[vact_VoV_key_name]->GetYaxis()->SetTitle("ln(I_{dark} @ 3V_{over})");
    }
    //
    for (auto current_status_value : status_to_quantity)
    {
        auto current_status = get<0>(current_status_value);
        auto current_value = get<1>(current_status_value);
        std::map<std::string, std::pair<float, float>> final_vbd_values;
        std::map<std::string, std::pair<float, float>> final_vact_values;
        std::map<std::string, std::pair<float, float>> final_vact_VoV_values;
        std::map<std::string, std::vector<std::pair<float, float>>> current_vbd_values;
        std::map<std::string, std::vector<std::pair<float, float>>> current_vact_values;
        std::map<std::string, std::vector<std::pair<float, float>>> current_vact_VoV_values;
        for (auto current_board : boards_for_scan)
        {
            //  Measure relevant quantities
            for (auto current_channel : database::all_channels)
            {
                auto key_name = TString(current_board) + "_" + TString(current_channel) + "_" + TString(current_status);
                auto current_iv = database::get_iv_scan(current_board, current_channel, current_status);
                if (!current_iv)
                    continue;
                current_iv->SetName(key_name);
                auto current_Vbd = measure_breakdown<1>(current_iv, 0.);
                if (TString(current_channel).Contains("A") && false)
                {
                    current_Vbd = measure_breakdown<0>(current_iv, 0.);
                }
                if (fabs(get<0>(current_Vbd) - reference_Vbd[database::channel_to_sensor[current_channel]]) > 3)
                    continue;
                auto current_Vact = calc_log(eval_with_errors(current_iv, reference_Vbd[database::channel_to_sensor[current_channel]] + 3));
                auto current_Vact_VoV = calc_log(eval_with_errors(current_iv, get<0>(current_Vbd) + 3));
                current_vbd_values[database::channel_to_sensor[current_channel]].push_back(current_Vbd);
                current_vact_values[database::channel_to_sensor[current_channel]].push_back(current_Vact);
                current_vact_VoV_values[database::channel_to_sensor[current_channel]].push_back(current_Vact_VoV);
            }
        }
        if (current_vbd_values.size() == 0)
            continue;
        for (auto [current_sensor, current_vbd_array] : current_vbd_values)
        {
            //  Calculate final values
            final_vbd_values[current_sensor] = {0, 0};
            final_vact_values[current_sensor] = {0, 0};
            final_vact_VoV_values[current_sensor] = {0, 0};
            auto iTer = -1;
            for (auto current_vbd : current_vbd_array)
            {
                iTer++;
                get<0>(final_vbd_values[current_sensor]) += get<0>(current_vbd_values[current_sensor].at(iTer)) / ((current_vbd_values[current_sensor]).size());
                get<0>(final_vact_values[current_sensor]) += get<0>(current_vact_values[current_sensor].at(iTer)) / ((current_vact_values[current_sensor]).size());
                get<0>(final_vact_VoV_values[current_sensor]) += get<0>(current_vact_VoV_values[current_sensor].at(iTer)) / ((current_vact_VoV_values[current_sensor]).size());
            }
            //  Calculate uncertainties
            auto error_square_Vbd = 0.;
            auto error_square_Vact = 0.;
            auto error_square_Vact_VoV = 0.;
            for (auto current_vbd : current_vbd_array)
            {
                error_square_Vbd += (get<0>(current_vbd_values[current_sensor].at(iTer)) - get<0>(final_vbd_values[current_sensor])) * (get<0>(current_vbd_values[current_sensor].at(iTer)) - get<0>(final_vbd_values[current_sensor]));
                error_square_Vact += (get<0>(current_vact_values[current_sensor].at(iTer)) - get<0>(final_vact_values[current_sensor])) * (get<0>(current_vact_values[current_sensor].at(iTer)) - get<0>(final_vact_values[current_sensor]));
                error_square_Vact_VoV += (get<0>(current_vact_VoV_values[current_sensor].at(iTer)) - get<0>(final_vact_VoV_values[current_sensor])) * (get<0>(current_vact_VoV_values[current_sensor].at(iTer)) - get<0>(final_vact_VoV_values[current_sensor]));
            }
            //  Assign final error
            get<1>(final_vbd_values[current_sensor]) = error_square_Vbd / ((current_vbd_values[current_sensor]).size());
            get<1>(final_vact_values[current_sensor]) = error_square_Vact / ((current_vact_values[current_sensor]).size());
            get<1>(final_vact_VoV_values[current_sensor]) = error_square_Vact_VoV / ((current_vact_VoV_values[current_sensor]).size());
        }
        for (auto [current_sensor, current_channels] : database::sensor_to_channels)
        {
            //  Recover graphs keys
            auto vbd_key_name = TString(current_sensor) + "_Vbd";
            auto vact_key_name = TString(current_sensor) + "_Vact";
            auto vact_VoV_key_name = TString(current_sensor) + "_VactVoV";
            //  Reference point
            auto current_point = _TGraphErrors[vbd_key_name]->GetN();
            //  Assign values and uncertainties
            //
            _TGraphErrors[vbd_key_name]->SetPoint(current_point, current_value, get<0>(final_vbd_values[current_sensor]));
            _TGraphErrors[vbd_key_name]->SetPointError(current_point, 0, TMath::Sqrt(get<1>(final_vbd_values[current_sensor])));
            //
            _TGraphErrors[vact_key_name]->SetPoint(current_point, 1. / (current_value * database::k_boltz_eV), get<0>(final_vact_values[current_sensor]));
            _TGraphErrors[vact_key_name]->SetPointError(current_point, 0, TMath::Sqrt(get<1>(final_vact_values[current_sensor])));
            //
            _TGraphErrors[vact_VoV_key_name]->SetPoint(current_point, 1. / (current_value * database::k_boltz_eV), get<0>(final_vact_VoV_values[current_sensor]));
            _TGraphErrors[vact_VoV_key_name]->SetPointError(current_point, 0, TMath::Sqrt(get<1>(final_vact_VoV_values[current_sensor])));
        }
    }
    //
    TFile *fOut = new TFile("fOut1516.root", "RECREATE");
    for (auto [key, value] : _TMultiGraph)
    {
        value->Write(key);
    }
    for (auto [key, value] : _TGraphErrors)
    {
        value->Write(key);
    }
    fOut->Close();
}