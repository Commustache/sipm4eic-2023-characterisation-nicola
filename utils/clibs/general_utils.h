#pragma once
//
string
get_environment_variable(string search_key)
{
  extern char **environ;
  int iTer = 0;
  string result = "./";
  while (environ[iTer])
  {
    string current_string = environ[iTer++];
    auto key_position = current_string.find(search_key);
    auto key_length = search_key.size();
    if (key_position < current_string.size() && key_position == 0 && current_string.substr(key_length, 1) == "=")
    {
      result = current_string.substr(key_length + key_position + 1);
    }
  }
  return result;
}
//
std::pair<float, float>
intersection(TF1 *left_function, TF1 *right_function, float start_value, float tolerance = 1.e-3, bool ascending = true)
{
  auto _currentX = start_value + (ascending ? -1 : +1);
  auto _currentIncrease = 1.;
  auto iTer = 0;
  auto stop_iter = 100;
  while (true)
  {
    iTer++;
    auto y1 = fabs(left_function->Eval(_currentX));
    auto y2 = fabs(right_function->Eval(_currentX));
    if (ascending ? y1 > y2 : y1 < y2)
    {
      if (fabs(y1 - y2) < tolerance)
      {
        return {_currentX, -1};
      }
      else
      {
        _currentIncrease *= 0.1;
        ascending = !ascending;
        if (ascending)
        {
          _currentX += _currentIncrease;
        }
        else
        {
          _currentX -= _currentIncrease;
        }
      }
    }
    if (ascending)
    {
      _currentX += _currentIncrease;
    }
    else
    {
      _currentX -= _currentIncrease;
    }
    if (iTer > stop_iter)
      break;
  }
  return {-1, -1};
}
//
std::pair<float, float>
intersection_with_errors(TF1 *left_function, TVirtualFitter *left_results, TF1 *right_function, TVirtualFitter *right_results, float start_value, float tolerance = 1.e-3, bool ascending = true)
{
  auto result = intersection(left_function, right_function, start_value, tolerance, ascending);
  double _xleftvalue[1] = {get<0>(result)};
  double _xlefterror[1] = {-1.};
  double _xrightvalue[1] = {get<0>(result)};
  double _xrighterror[1] = {-1.};
  left_results->GetConfidenceIntervals(1, 1, _xleftvalue, _xlefterror, 0.683);
  right_results->GetConfidenceIntervals(1, 1, _xrightvalue, _xrighterror, 0.683);
  auto low_left_bound = left_function->GetX(_xrightvalue[0] - _xrighterror[0]);
  auto high_left_bound = left_function->GetX(_xrightvalue[0] + _xrighterror[0]);
  auto low_right_bound = right_function->GetX(_xleftvalue[0] - _xlefterror[0]);
  auto high_right_bound = right_function->GetX(_xleftvalue[0] + _xlefterror[0]);
  get<1>(result) = 0.5 * sqrt((high_left_bound - low_left_bound) * (high_left_bound - low_left_bound) + (high_right_bound - low_right_bound) * (high_right_bound - low_right_bound));
  return result;
}
//
TGraphErrors *
derivative_function(TF1 *fTarget, double xmin, double xmax, int npoints)
{
  TGraphErrors *result = new TGraphErrors();
  for (int iPnt = 0; iPnt < npoints; iPnt++)
  {
    auto x_value = xmin + (xmax - xmin) * iPnt / (npoints - 1);
    auto y_value = fTarget->Derivative(x_value);
    result->SetPoint(iPnt, x_value, y_value);
  }
  return result;
}
//
template <bool use_first_as_denominator = true>
std::pair<float, float>
relative_ratio(std::pair<float, float> first, std::pair<float, float> second)
{
  auto numerator = get<0>(first) - get<0>(second);
  auto denominator = use_first_as_denominator ? get<0>(first) : get<0>(second);
  auto numerator_e = use_first_as_denominator ? get<1>(second) * (-1 / (get<0>(first))) : get<1>(first) * (-1 / (get<0>(second)));
  auto denominator_e = use_first_as_denominator ? get<1>(first) * (get<0>(second) / (get<0>(first) * get<0>(first))) : get<1>(second) * (get<0>(first) / (get<0>(second) * get<0>(second)));
  return {numerator / denominator, sqrt(numerator_e * numerator_e + denominator_e * denominator_e)};
}
//
std::vector<float>
build_my_vector(float min, float max, int npoints)
{
  std::vector<float> result;
  for (auto iPnt = 0; iPnt < npoints; iPnt++)
  {
    result.push_back(min + iPnt * (max - min) / (npoints - 1));
  }
  return result;
}
//
std::pair<float, float>
calc_ratio(std::pair<float, float> numerator, std::pair<float, float> denominator)
{
  std::pair<float, float> result;
  get<0>(result) = get<0>(numerator) / get<0>(denominator);
  get<1>(result) = get<0>(result) * sqrt(get<1>(numerator) * get<1>(numerator) / (get<0>(numerator) * get<0>(numerator)) + get<1>(denominator) * get<1>(denominator) / (get<0>(denominator) * get<0>(denominator)));
  return result;
}
//
std::pair<float, float>
calc_log(std::pair<float, float> argument)
{
  std::pair<float, float> result;
  get<0>(result) = TMath::Log(get<0>(argument));
  get<1>(result) = get<1>(argument)/get<0>(argument);
  return result;
}
//
std::pair<float, float>
calc_heal(std::pair<float, float> current_step, std::pair<float, float> irr_step, std::pair<float, float> new_step)
{
  std::pair<float, float> result;
  get<0>(result) = (get<0>(current_step) - get<0>(new_step)) / (get<0>(irr_step) - get<0>(new_step));
  auto current_step_error = get<1>(current_step) / (get<0>(irr_step) - get<0>(new_step));
  auto irr_step_error = get<1>(irr_step) * (get<0>(new_step) - get<0>(current_step)) / ((get<0>(irr_step) - get<0>(new_step)) * (get<0>(irr_step) - get<0>(new_step)));
  auto new_step_error = get<1>(new_step) * (get<0>(current_step) - get<0>(irr_step)) / ((get<0>(irr_step) - get<0>(new_step)) * (get<0>(irr_step) - get<0>(new_step)));
  get<1>(result) = sqrt(current_step_error * current_step_error + irr_step_error * irr_step_error + new_step_error * new_step_error);
  return result;
}