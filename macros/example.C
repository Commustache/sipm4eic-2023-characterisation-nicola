/**
    this example plots a IV and a DCR scan for one channel
    the data and the database file are assumed to be located in
    "/home/preghenella/EIC/sipm4eic-2023-characterisation/data"
**/

#include "../source/database.h"
#include "../utils/clibs/analysis_utils.h"

//  --- Quality checks
std::vector<TString> quality_check = {
    //  General quantities
    "VbdDiff", //  --- Vbd
    "Gain",    //  --- Gain
    //  Dark current
    "OV_Current",   // --- Absolute
    "OV_A_Heal",    // --- Heal from TIFPA-IRR1
    "OV_A_NewDiff", // --- Difference to NEW
    //  Dark counts
    "OV_DCR",        // --- Absolute
    "OV_DCR_Heal",   // --- Heal from TIFPA-IRR1
    "OV_DCR_NewDiff" // --- Difference to NEW
};

template <bool logy = false,
          bool top = false>
void plot_my_sensors(std::map<TString, std::map<TString, std::vector<TString>>> list_of_sensor_graphs, std::map<TString, TGraphErrors *> &_TGraphErrors, std::map<TString, TMultiGraph *> &_TMultiGraph, std::vector<std::pair<TString, TString>> plot_quantity, TString TMultiGraph_quantity, TString title = "")
{
  //  --- Full TMultiGraph
  _TMultiGraph[TMultiGraph_quantity] = new TMultiGraph();
  _TMultiGraph[TMultiGraph_quantity]->SetNameTitle(TMultiGraph_quantity, title);
  //  --- Make MultiGraph
  auto iTer = -1;
  for (auto current_sensor : sensors)
  {
    iTer++;
    auto jTer = -1;
    for (auto current_quantity : plot_quantity)
    {
      jTer++;
      std::vector<TGraphErrors *> temp;
      for (auto current_graph : list_of_sensor_graphs[get<0>(current_quantity)][sensors_full.at(iTer)])
      {
        temp.push_back(_TGraphErrors[current_graph]);
      }
      _TGraphErrors[current_sensor + get<0>(current_quantity)] = graphutils::average_same_x(temp)["rms"];
      _TGraphErrors[current_sensor + get<0>(current_quantity)]->SetName(current_sensor + get<0>(current_quantity));
      set_style(_TGraphErrors[current_sensor + get<0>(current_quantity)], quantity_marker.at(jTer), sensors_color.at(iTer), 2);
      _TMultiGraph[TMultiGraph_quantity]->Add(_TGraphErrors[current_sensor + get<0>(current_quantity)]);
    }
  }
  //  --- TCanvas
  TLegend *legend = new TLegend(0.5, top ? 0.85 : 0.15, 0.85, top ? 0.75 : 0.25);
  legend->SetNColumns(min(3, (int)(plot_quantity.size())));
  legend->SetFillColorAlpha(0., 0.);
  legend->SetLineColorAlpha(0., 0.);
  legend->SetFillStyle(0);
  iTer = -1;
  for (auto current_quantity : plot_quantity)
  {
    iTer++;
    auto current_marker = new TMarker(0., 0., quantity_marker.at(iTer));
    current_marker->SetMarkerSize(2);
    legend->AddEntry(current_marker, get<1>(current_quantity));
  }
  //  --- --- plot
  TCanvas *cVbd = new TCanvas(TMultiGraph_quantity, TMultiGraph_quantity, 1035, 1000);
  gPad->SetLogy(logy);
  gPad->SetTickx();
  gPad->SetTicky();
  gPad->SetTopMargin(0.125);
  gPad->SetBottomMargin(0.125);
  gPad->SetRightMargin(0.125);
  gPad->SetLeftMargin(0.150);
  _TMultiGraph[TMultiGraph_quantity]->Draw("AP");
  legend->Draw("SAME");
  TLatex *llatex = new TLatex();
  iTer = -1;
  for (auto current_sensor : sensors_legend)
  {
    iTer++;
    llatex->SetTextColor(sensors_color.at(iTer));
    llatex->DrawLatexNDC(0.15 + iTer * 0.15, 0.92, current_sensor);
  }
  cVbd->SaveAs(TMultiGraph_quantity + TString(".pdf"));
}

void plot_heal_factor(std::map<TString, TMultiGraph *> &_TMultiGraph, TString target_multigraph)
{
  //  Get the first two points as side reference
  auto reference_histograms = switch_to_label(_TMultiGraph[target_multigraph], {"NEW", "TIFPA-IRR1"});
  //
  //  Result Canvas
  TCanvas *result = new TCanvas("result", "result", 1000, 1000);
  //
  //  Remove first two points of TGraphErrors
  auto list_of_graphs = _TMultiGraph[target_multigraph]->GetListOfGraphs();
  auto iTer = -1;
  while (true)
  {
    iTer++;
    ((TGraphErrors *)(list_of_graphs->At(iTer)))->Sort();
    ((TGraphErrors *)(list_of_graphs->At(iTer)))->RemovePoint(0);
    ((TGraphErrors *)(list_of_graphs->At(iTer)))->RemovePoint(0);
    if (list_of_graphs->At(iTer) == list_of_graphs->Last())
      break;
  }
  //  Left side (Reference) pad
  TPad *left_pad = new TPad("left_pad", "left_pad", 0.0, 0.0, 0.3, 1.0);
  left_pad->SetRightMargin(0);
  left_pad->Draw();
  left_pad->cd();
  left_pad->SetLogy();
  gStyle->SetOptStat(0);
  for (auto current_histogram : reference_histograms)
  {
    current_histogram->SetMaximum(2);
    current_histogram->SetMinimum(1.e-4);
    current_histogram->Draw("SAME");
  }
  //  --- Go back to main canvas
  result->cd();
  //
  //  Right side (Data) pad
  TPad *right_pad = new TPad("right_pad", "right_pad", 0.3, 0.0, 1.0, 1.0);
  right_pad->SetLeftMargin(0);
  right_pad->Draw();
  right_pad->cd();
  right_pad->SetLogy();
  right_pad->SetLogx();
  gStyle->SetOptStat(0);
  _TMultiGraph[target_multigraph]->SetMinimum(1.e-4);
  _TMultiGraph[target_multigraph]->SetMaximum(2);
  _TMultiGraph[target_multigraph]->GetXaxis()->SetRangeUser(10., 10000.);
  _TMultiGraph[target_multigraph]->Draw("AP");
  //
  result->SaveAs("test.pdf");
}

void example()
{
  gErrorIgnoreLevel = 6001;
  std::map<TString, TH1F *> _TH1F;
  std::map<TString, TH1F *> _TH2F;
  std::map<TString, TMultiGraph *> _TMultiGraph;
  std::map<TString, TGraphErrors *> _TGraphErrors;
  //
  database::basedir = get_environment_variable("SIPM4EIC_DATA");
  database::read_database(database::basedir + "/database.txt");
  //
  database::check_all_plots_loaded();
  //
  return;
  // gROOT->SetBatch();
  database::calculate_breakdown_voltages();
  // gROOT->SetBatch(false);
  //
  std::vector<std::string> all_considered_board;
  std::vector<std::string> all_considered_channels;
  auto all_Vbd = database::get_all_breakdown_voltages();
  //
  for (auto [current_board, board_map] : all_Vbd)
  {
    for (auto [current_channel, status_map] : board_map)
    {
      auto current_iv_NEW = database::get_iv_scan(current_board, current_channel, "NEW");
      if (!current_iv_NEW)
        continue;
      if (!current_iv_NEW->GetN())
        continue;
      auto current_dcr_NEW = database::get_dcr_vbias_scan(current_board, current_channel, "NEW");
      if (!current_dcr_NEW)
        continue;
      if (!current_dcr_NEW->GetN())
        continue;
      auto current_iv_IRR = database::get_iv_scan(current_board, current_channel, "TIFPA-IRR1");
      if (!current_iv_IRR)
        continue;
      if (!current_iv_IRR->GetN())
        continue;
      auto current_dcr_IRR = database::get_dcr_vbias_scan(current_board, current_channel, "TIFPA-IRR1");
      if (!current_dcr_IRR)
        continue;
      if (!current_dcr_NEW->GetN())
        continue;
      for (auto [current_status, breakdown_voltage] : status_map)
      {
        auto current_iv = database::get_iv_scan(current_board, current_channel, current_status);
        if (!current_iv)
          continue;
        if (!current_iv->GetN())
          continue;
        auto current_dcr = database::get_dcr_vbias_scan(current_board, current_channel, current_status);
        if (!current_dcr)
          continue;
        if (!current_dcr->GetN())
          continue;
        auto current_gain = database::get_gain(current_board, current_channel, current_status);
        if (!current_gain)
          continue;
        if (!current_gain->GetN())
          continue;
        if (isnan(get<0>(breakdown_voltage)))
          continue;
        auto key_name = TString(current_board) + "_" + TString(current_channel);
        //  Check the TGraphErrors for the checks exists
        if (_TGraphErrors.find(key_name + "_" + quality_check.at(0)) == _TGraphErrors.end())
        {
          for (auto current_check : quality_check)
          {
            _TGraphErrors[key_name + "_" + current_check] = new TGraphErrors();
            _TGraphErrors[key_name + "_" + current_check]->SetName(key_name + "_" + current_check);
          }
        }
        //  Recover needed quantities for all quality checks
        //  --- Breakdown voltages
        auto breakdown_IRR = all_Vbd[current_board][current_channel]["TIFPA-IRR1"];
        auto breakdown_current = breakdown_voltage;
        //  --- Dark currents
        auto VoV3_A_NEW = eval_with_errors(shift_with_vbd<false, -1>(current_iv_NEW, get<0>(breakdown_IRR)), 3.);
        auto VoV3_A_IRR = eval_with_errors(shift_with_vbd<false, -1>(current_iv_IRR, get<0>(breakdown_IRR)), 3.);
        auto VoV3_A_current = eval_with_errors(shift_with_vbd<false, -1>(current_iv, get<0>(breakdown_current)), 3.);
        //  --- --- Derived quantities
        auto VoV3_A_Heal = calc_heal(VoV3_A_current, VoV3_A_IRR, VoV3_A_NEW);
        auto VoV3_A_NewRatio = calc_ratio(VoV3_A_current, VoV3_A_NEW);
        //  --- Dark counts
        auto VoV3_DCR_NEW = eval_with_errors(shift_with_vbd<false, -1>(current_dcr_NEW, get<0>(breakdown_IRR)), 3.);
        auto VoV3_DCR_IRR = eval_with_errors(shift_with_vbd<false, -1>(current_dcr_IRR, get<0>(breakdown_IRR)), 3.);
        auto VoV3_DCR_current = eval_with_errors(shift_with_vbd<false, -1>(current_dcr, get<0>(breakdown_current)), 3.);
        //  --- --- Derived quantities
        auto VoV3_DCR_Heal = calc_heal(VoV3_DCR_current, VoV3_DCR_IRR, VoV3_DCR_NEW);
        auto VoV3_DCR_NewRatio = calc_ratio(VoV3_DCR_current, VoV3_DCR_NEW);
        //
        //  Current point set
        auto current_point = _TGraphErrors[key_name + "_" + quality_check.at(0)]->GetN();
        //  --- Sensor properties
        _TGraphErrors[key_name + TString("_VbdDiff")]->SetPoint(current_point, database::status_to_time[current_status], get<0>(breakdown_current) - get<0>(breakdown_IRR));
        _TGraphErrors[key_name + TString("_VbdDiff")]->SetPointError(current_point, database::status_to_time[current_status] * 0.01, sqrt(get<1>(breakdown_current) * get<1>(breakdown_current) + get<1>(breakdown_IRR) * get<1>(breakdown_IRR)));
        /*
        _TGraphErrors[key_name + TString("_Gain")]->SetPoint(current_point, database::status_to_time[current_status], get<0>(VoV_gain));
        _TGraphErrors[key_name + TString("_Gain")]->SetPointError(current_point, database::status_to_time[current_status] * 0.01, get<1>(VoV_gain));
        */
        //  --- Dark current
        _TGraphErrors[key_name + TString("_OV_Current")]->SetPoint(current_point, database::status_to_time[current_status], get<0>(VoV3_A_current));
        _TGraphErrors[key_name + TString("_OV_Current")]->SetPointError(current_point, database::status_to_time[current_status] * 0.01, get<1>(VoV3_A_current));
        _TGraphErrors[key_name + TString("_OV_A_Heal")]->SetPoint(current_point, database::status_to_time[current_status], get<0>(VoV3_DCR_Heal));
        _TGraphErrors[key_name + TString("_OV_A_Heal")]->SetPointError(current_point, database::status_to_time[current_status] * 0.01, get<1>(VoV3_DCR_Heal));
        _TGraphErrors[key_name + TString("_OV_A_NewDiff")]->SetPoint(current_point, database::status_to_time[current_status], get<0>(VoV3_A_current) - get<0>(VoV3_A_NEW));
        _TGraphErrors[key_name + TString("_OV_A_NewDiff")]->SetPointError(current_point, database::status_to_time[current_status] * 0.01, sqrt(get<1>(VoV3_A_current) * get<1>(VoV3_A_current) + get<1>(VoV3_A_NEW) * get<1>(VoV3_A_NEW)));
        //  --- Dark count
        _TGraphErrors[key_name + TString("_OV_DCR")]->SetPoint(current_point, database::status_to_time[current_status], get<0>(VoV3_DCR_current));
        _TGraphErrors[key_name + TString("_OV_DCR")]->SetPointError(current_point, database::status_to_time[current_status] * 0.01, get<1>(VoV3_DCR_current));
        _TGraphErrors[key_name + TString("_OV_DCR_Heal")]->SetPoint(current_point, database::status_to_time[current_status], get<0>(VoV3_DCR_Heal));
        _TGraphErrors[key_name + TString("_OV_DCR_Heal")]->SetPointError(current_point, database::status_to_time[current_status] * 0.01, get<1>(VoV3_DCR_Heal));
        _TGraphErrors[key_name + TString("_OV_DCR_NewDiff")]->SetPoint(current_point, database::status_to_time[current_status], get<0>(VoV3_DCR_current) - get<0>(VoV3_DCR_NEW));
        _TGraphErrors[key_name + TString("_OV_DCR_NewDiff")]->SetPointError(current_point, database::status_to_time[current_status] * 0.01, sqrt(get<1>(VoV3_DCR_current) * get<1>(VoV3_DCR_current) + get<1>(VoV3_DCR_NEW) * get<1>(VoV3_DCR_NEW)));
        //
        if (std::find(all_considered_board.begin(), all_considered_board.end(), current_board) == all_considered_board.end())
          all_considered_board.push_back(current_board);
        if (std::find(all_considered_channels.begin(), all_considered_channels.end(), current_channel) == all_considered_channels.end())
          all_considered_channels.push_back(current_channel);
      }
    }
  }
  //
  std::map<TString, std::map<TString, std::vector<TString>>> list_of_sensor_graphs;
  std::vector<std::pair<TString, TString>> list_of_observables = {{"spare", "16"}, {"forward", "13"}, {"reverse", "14"}, {"pann_1", "3"}, {"pann_2", "4"}, /*{"pann_3", "17"}, {"pann_3", "18"},*/ {"online_fwd", "11"}, {"online_rev", "12"}};
  //
  for (auto current_channel : all_considered_channels)
  {
    for (auto current_observable : list_of_observables)
    {
      for (auto current_check : quality_check)
      {
        auto key_name = get<1>(current_observable) + "_" + TString(current_channel);
        list_of_sensor_graphs[current_check + "_" + get<0>(current_observable)][database::channel_to_sensor[current_channel]].push_back(key_name + "_" + current_check);
        if (!_TGraphErrors[key_name + "_" + current_check])
          cout << "lost _TGraphErrors[" << (key_name + "_" + current_check).Data() << "]: " << endl;
      }
    }
  }
  //
  //  Quantity ask
  std::vector<std::tuple<TString, std::pair<bool, bool>, TString>> list_of_standard_plots = {
      //  --- Sensor properties
      {"VbdDiff_", {false, false}, ";Annealing time (min);"},
      {"Gain_", {false, true}, ";Annealing time (min);Sensor pseudo-gain"},
      //  Dark current
      {"OV_Current_", {true, true}, ";Annealing time (min);Dark current @ 3VoV (A)"},
      {"OV_A_Heal_", {true, true}, ";Annealing time (min);Heal factor @ 3VoV w.r.t to IRR sensor (Current)"},
      {"OV_A_NewDiff_", {true, true}, ";Annealing time (min);Dark current difference w.r.t to NEW sensor @ 3VoV (A)"},
      //  Dark counts
      {"OV_DCR_", {true, true}, ";Annealing time (min);Dark counts @ 3VoV (Hz)"},
      {"OV_DCR_Heal_", {true, true}, ";Annealing time (min);Heal factor @ 3VoV factor w.r.t to IRR sensor (DCR)"},
      {"OV_DCR_NewDiff_", {true, true}, ";Annealing time (min);Dark counts difference w.r.t to NEW sensor @ 3VoV (Hz)"}};
  //
  std::vector<std::pair<TString, std::vector<std::pair<TString, TString>>>> list_of_plot_quantities = {
      {"onlinerevforw", {{"online_fwd", "on. forward"}, {"online_rev", "on. reverse"}}},
      {"preventive", {{"pann_1", "PANN 1"}, {"pann_2", "PANN 2"}, {"spare", "no ann."}}},
      {"revforw", {{"forward", "forward"}, {"reverse", "reverse"}}},
      {"onlinerevforw_compare", {{"online_fwd", "on. forward"}, {"online_rev", "on. reverse"}, {"forward", "forward"}, {"reverse", "reverse"}}},
      {"fwd_compare", {{"online_fwd", "on. forward"}, {"forward", "off. forward"}}},
      {"rev_compare", {{"online_rev", "on. reverse"}, {"reverse", "off. reverse"}}}};
  //
  for (auto current_request : list_of_plot_quantities)
  {
    for (auto current_target : list_of_standard_plots)
    {
      std::vector<std::pair<TString, TString>> current_list_plots_keyname;
      for (auto current_keyname : get<1>(current_request))
      {
        current_list_plots_keyname.push_back({get<0>(current_target) + get<0>(current_keyname), get<1>(current_keyname)});
      }
      if (get<1>(current_target).first)
      {
        if (get<1>(current_target).second)
        {
          plot_my_sensors<true, true>(list_of_sensor_graphs, _TGraphErrors, _TMultiGraph, current_list_plots_keyname, get<0>(current_target) + get<0>(current_request), get<2>(current_target));
        }
        else
        {
          plot_my_sensors<true, false>(list_of_sensor_graphs, _TGraphErrors, _TMultiGraph, current_list_plots_keyname, get<0>(current_target) + get<0>(current_request), get<2>(current_target));
        }
      }
      else
      {
        if (get<1>(current_target).second)
        {
          plot_my_sensors<false, true>(list_of_sensor_graphs, _TGraphErrors, _TMultiGraph, current_list_plots_keyname, get<0>(current_target) + get<0>(current_request), get<2>(current_target));
        }
        else
        {
          plot_my_sensors<false, false>(list_of_sensor_graphs, _TGraphErrors, _TMultiGraph, current_list_plots_keyname, get<0>(current_target) + get<0>(current_request), get<2>(current_target));
        }
      }
    }
  }
  plot_heal_factor(_TMultiGraph, "OV_DCR_Heal_fwd_compare");
  //
  TFile *out = new TFile("out.root", "RECREATE");
  for (auto [key, value] : _TH1F)
  {
    value->Write();
  }
  for (auto [key, value] : _TGraphErrors)
  {
    value->Write();
  }
  for (auto [key, value] : _TMultiGraph)
  {
    value->Write();
  }
  out->Close();
}