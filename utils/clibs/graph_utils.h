#pragma once
//
std::pair<float, float>
eval_with_errors(TGraphErrors *gTarget, float _xtarget)
{
    auto iPnt = 0;
    auto current_x = 1.;
    auto previous_x = 1.;
    auto interpolated_y = -1.;
    auto interpolated_ey = -1.;
    for (iPnt = 1; iPnt < gTarget->GetN() - 1; iPnt++)
    {
        current_x = gTarget->GetPointX(iPnt);
        previous_x = gTarget->GetPointX(iPnt - 1);
        if ((current_x >= _xtarget) && (previous_x <= _xtarget))
        {
            break;
        }
    }
    if (iPnt == gTarget->GetN() - 1)
    {
        return {interpolated_y, interpolated_ey};
    }
    auto coeff_y1 = (_xtarget - previous_x) / (current_x - previous_x);
    auto coeff_y0 = (current_x - _xtarget) / (current_x - previous_x);
    interpolated_y = coeff_y1 * gTarget->GetPointY(iPnt) + coeff_y0 * gTarget->GetPointY(iPnt - 1);
    interpolated_ey = sqrt((coeff_y0 * gTarget->GetErrorY(iPnt)) * (coeff_y0 * gTarget->GetErrorY(iPnt)) + (coeff_y1 * gTarget->GetErrorY(iPnt - 1)) * (coeff_y1 * gTarget->GetErrorY(iPnt - 1)));
    return {interpolated_y, interpolated_ey};
}
//
void set_style(TGraphErrors *gTarget, int marker_style, int marker_color, int marker_size, int line_style = 1, int line_color = kBlack, int line_size = 1)
{
    gTarget->SetMarkerStyle(marker_style);
    gTarget->SetMarkerColor(marker_color);
    gTarget->SetMarkerSize(marker_size);
    gTarget->SetLineStyle(line_style);
    gTarget->SetLineColor(line_color);
    gTarget->SetLineWidth(line_size);
}
//
TH1F *
switch_to_label(TGraphErrors *gTarget, std::vector<TString> labels, bool boolt)
{
    auto result = new TH1F("switch_to_label", "", labels.size(), 0., labels.size());
    auto iTer = -1;
    result->SetMarkerStyle(gTarget->GetMarkerStyle());
    result->SetMarkerColor(gTarget->GetMarkerColor());
    result->SetMarkerSize(gTarget->GetMarkerSize());
    result->SetLineStyle(gTarget->GetLineStyle());
    result->SetLineColor(gTarget->GetLineColor());
    result->SetLineWidth(gTarget->GetLineWidth());
    for (auto current_label : labels)
    {
        iTer++;
        if (iTer > gTarget->GetN() + 1)
            return result;
        auto current_y = gTarget->GetY()[iTer];
        auto current_ey = gTarget->GetEY()[iTer];
        result->SetBinContent(iTer + 1 + boolt, current_y);
        result->SetBinError(iTer + 1 + boolt, current_ey);
        result->GetXaxis()->SetBinLabel(iTer + 1, current_label);
    }
    return result;
}
//
std::vector<TH1F *>
switch_to_label(TMultiGraph *gTarget, std::vector<TString> labels, bool boolt = false)
{
    std::vector<TH1F *> result;
    auto list_of_graphs = gTarget->GetListOfGraphs();
    auto iTer = -1;
    while (true)
    {
        iTer++;
        result.push_back(switch_to_label((TGraphErrors *)(list_of_graphs->At(iTer)), labels, boolt));
        if (list_of_graphs->At(iTer) == list_of_graphs->Last())
            break;
    }
    return result;
}
//
void set_log_binning(TH1 *h)
{
    TAxis *axis = h->GetXaxis();
    Int_t bins = axis->GetNbins();
    Axis_t from = TMath::Log10(axis->GetXmin());
    Axis_t to = TMath::Log10(axis->GetXmax());
    Axis_t width = (to - from) / bins;
    Axis_t *new_bins = new Axis_t[bins + 1];
    for (int i = 0; i <= bins; i++)
    {
        new_bins[i] = TMath::Power(10, from + i * width);
    }
    axis->Set(bins, new_bins);
    delete[] new_bins;
}
/*
void set_log_binning(TH1 *hTarget)
{
    TAxis *current_axis = hTarget->GetXaxis();
    Int_t bins = current_axis->GetNbins();
    Axis_t from = current_axis->GetXmin();
    Axis_t to = current_axis->GetXmax();
    Axis_t width = (to - from) / bins;
    Axis_t *new_bins = new Axis_t[bins + 1];
    for (int i = 0; i <= bins; i++)
        new_bins[i] = TMath::Power(10, from + i * width);
    current_axis->Set(bins, new_bins);
    delete[] new_bins;
}*/