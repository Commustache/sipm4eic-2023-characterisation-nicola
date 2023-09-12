#include "../source/database.h"
#include "../utils/clibs/analysis_utils.h"
//
void temperature_scan_final()
{
    TFile *fInputNEW = new TFile("/Users/nrubini/Analysis/ePIC/sipm4eic-2023-characterisation/fOut3132.root");
    TFile *fInputIRR = new TFile("/Users/nrubini/Analysis/ePIC/sipm4eic-2023-characterisation/fOut1516.root");
    auto graph_1350_NEW = (TGraphErrors *)(fInputNEW->Get("Hamamatsu S13361-3050_VactVoV"));
    auto graph_1350_IRR = (TGraphErrors *)(fInputIRR->Get("Hamamatsu S13361-3050_VactVoV"));
    auto graph_1375_NEW = (TGraphErrors *)(fInputNEW->Get("Hamamatsu S13361-3075_VactVoV"));
    auto graph_1375_IRR = (TGraphErrors *)(fInputIRR->Get("Hamamatsu S13361-3075_VactVoV"));
    auto graph_1450_NEW = (TGraphErrors *)(fInputNEW->Get("Hamamatsu S14161-3050_VactVoV"));
    auto graph_1450_IRR = (TGraphErrors *)(fInputIRR->Get("Hamamatsu S14161-3050_VactVoV"));
    //  Prepare fit
    TF1 *pol1_bf_new = new TF1("pol1", "pol1");
    TF1 *pol1_af_new = new TF1("pol1", "pol1");
    TF1 *pol1_bf_irr = new TF1("pol1", "pol1");
    TF1 *pol1_af_irr = new TF1("pol1", "pol1");
    TLatex * llatex = new TLatex();
    //  S1350
    TCanvas *c1350 = new TCanvas();
    graph_1350_NEW->SetMinimum(-22);
    graph_1350_NEW->SetMaximum(-10);
    graph_1350_NEW->Draw("APE");
    graph_1350_IRR->Draw("SAMEPE");
    graph_1350_NEW->Fit(pol1_bf_new, "Q", "", 37., 42.);
    pol1_bf_new->SetRange(0, 42);
    pol1_bf_new->SetLineColor(kGreen-2);
    pol1_bf_new->SetLineStyle(kSolid);
    pol1_bf_new->Draw("SAME");
    graph_1350_NEW->Fit(pol1_af_new, "Q", "", 42., 50.);
    pol1_af_new->SetRange(42, 100);
    pol1_af_new->SetLineColor(kGreen-2);
    pol1_af_new->SetLineStyle(kSolid);
    pol1_af_new->Draw("SAME");
    graph_1350_IRR->Fit(pol1_bf_irr, "Q", "", 37., 42.);
    pol1_bf_irr->SetRange(0, 42);
    pol1_bf_irr->SetLineColor(kGreen-2);
    pol1_bf_irr->SetLineStyle(kDashed);
    pol1_bf_irr->Draw("SAME");
    graph_1350_IRR->Fit(pol1_af_irr, "Q", "", 42., 50.);
    pol1_af_irr->SetRange(42, 100);
    pol1_af_irr->SetLineColor(kGreen-2);
    pol1_af_irr->SetLineStyle(kDashed);
    pol1_af_irr->Draw("SAME");
    llatex->DrawLatexNDC(0.15,0.85,"HAMA1350");
    llatex->DrawLatexNDC(0.12,0.20,Form("Activ. E NEW: %.2f#pm%.2f",-pol1_bf_new->GetParameter(1),pol1_bf_new->GetParError(1)));
    llatex->DrawLatexNDC(0.12,0.15,Form("Activ. E IRR: %.2f#pm%.2f",-pol1_bf_irr->GetParameter(1),pol1_bf_irr->GetParError(1)));
    llatex->DrawLatexNDC(0.50,0.85,Form("Activ. E NEW: %.2f#pm%.2f",-pol1_af_new->GetParameter(1),pol1_af_new->GetParError(1)));
    llatex->DrawLatexNDC(0.50,0.80,Form("Activ. E IRR: %.2f#pm%.2f",-pol1_af_irr->GetParameter(1),pol1_af_irr->GetParError(1)));

    //DrawLatexNDC(0.15,0.75,"FIT slope: 54.33#pm0.06mV/K")
}