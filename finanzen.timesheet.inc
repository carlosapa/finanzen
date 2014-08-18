<?php 

/**
 * This function outputs a timesheet month navigator
 */
function _finanzen_timesheet_month_navigator($date)
{ 
  $current_date = time ();
  $current_first_day_month = _finanzen_first_day_month($current_date);
  global $user;
  $user_first_day_month = _finanzen_first_day_month($user->created);
  
  $content .= '<div class="navigator">';
  
  $content .= '<div class="navigator-back"><a href="/timesheet"> &#8592; Back </a></div>';
  
  $content .= '<div class="navigator-date">';
  $content .= date('F Y', $date);
  $content .= '  <a href="/timesheet/edit-month/' . $date . '"><img class="edit-pencil" src="/' 
              . drupal_get_path('module', 'finanzen') . '/img/pencil.png" />edit all</a>';
  $content .= "</div>";
  
  $content .= '<div class="navigator-next">';
  if ($date < $current_first_day_month) {
	$content .= '<a title="Go to next month" href="/timesheet/month/' 
	            . _finanzen_get_first_day_next_month($date) . '"> &#8811; </a>';
  }
  else { $content .= '<span class="muted"> &#8811; </span>'; }
  $content .= "</div>";
  
  $content .= '<div class="navigator-prev">';
  if ($date > $user_first_day_month) {
  	$content .= '<a title="Go to previous month" href="/timesheet/month/' 
  	            . _finanzen_get_first_day_previous_month($date) . '"> &#8810; </a>';
  }
  else { $content .= '<span class="muted"> &#8810; </span>'; }
  $content .= "</div>";
  
  $content .= "</div>"; // class="navigator"
   
  return $content;     
} 

/**
 * This function outputs a timesheet edit month navigator
 */
function _finanzen_timesheet_edit_month_navigator ( $date ) 
{
  $content .= '<div class="navigator">';
  $content .= '<div class="navigator-back"><a href="/timesheet/month/' . $date . '"> &#8592; Back </a></div>';
  $content .= '<div class="navigator-date">' . date('F Y', $date) . '</div>';
  $content .= '</div>';
  
  return $content;
}

/**
 * This function outputs a timesheet edit month navigator
 */
function _finanzen_timesheet_insert_update_navigator ( $date ) 
{
  //$month = date('m', $date);
  //$year = date('Y', $date);
  //$first_day_month = mktime(0,0,0, $month, 1, $year);
  
  $first_day_month = _finanzen_first_day_month($date);
  
  $content .= '<div class="navigator">';
  $content .= '<div class="navigator-back"><a href="/timesheet/month/' . $first_day_month . '"> &#8592; Back </a></div>';
  $content .= '</div>';
  
  return $content;
}

/**
 * Mainpage for Timesheet feature
 */
function finanzen_timesheet() {

  $current_date = time ();
  
  global $user;
  
  if (empty($user->og_groups)) {
    $content .= 'You do not belong to any project. Go and <a href="og">join a project</a>.';
  }
  else {
  
    $first_day_month = _finanzen_first_day_month($user->created);
  
    $header = array('date' => t("Date"), 'summary' => t("Summary"), 'total' => t("Total"), 'edit' => t("Edit"), );

    while ($first_day_month < $current_date) {
   	  $row = array('date' => date('F Y', $first_day_month),
   	             'summary' => _finanzen_reports_summary_user_month($user->uid, $first_day_month, $total),
   	             'total' => round($total, 2) . ' hours',
   	             'edit' => '<a href="/timesheet/month/' . $first_day_month . '"><img class="edit-pencil" src="/'
                           . drupal_get_path('module', 'finanzen') . '/img/pencil.png" /></a>');
	  $rows[] = $row;
	  $first_day_month = _finanzen_get_first_day_next_month($first_day_month);
	  $total = 0;
    }
  
    $rows = array_reverse($rows);
  
    $content .= '<p class="month-report-title"><b>Monthly Report for ' . $user->name . '</b></p>';
    $content .= theme_table($header, $rows, array('class' => 'finanzen'));
  }
  
  return $content;
}

/**
 * Users Timesheet for the given month -> $first_day_month
 */
function finanzen_timesheet_month( $first_day_month ) {

  global $user;
  
  $month = date('m', $first_day_month);
  $year = date('Y', $first_day_month);
  
  $content .= _finanzen_timesheet_month_navigator($first_day_month);
  
  $number_days = _finanzen_days_in_month($month, $year);

  $all_projects = $user->og_groups;
  
  $header = array('date' => array('data' => t("Date"),'class' => 'column_date'));
  
  foreach ($all_projects as $project) {
    $project_code = _finanzen_get_project_code($project[nid]);
    $header[$project_code] = array('data' => $project_code, 'class' => 'column_project');
  }
  
  $rows = array ();
  
  for ($i = 1; $i <= $number_days; $i++) {
    $day = mktime(0,0,0,$month, $i, $year);
    $rowdata = array ();
    $rowdata['date'] = date('d.m.Y, D', $day);
        
    foreach ($all_projects as $project) {
      $query = "SELECT fuhp.nid, fuhp.hours FROM {finanzen_timesheet_worker_hours_projects} fuhp 
                WHERE fuhp.date=%d AND fuhp.uid=%d AND fuhp.project_id=%d";
      $queryResult = db_query ( $query, $day, $user->uid, $project[nid] );
      $resource = db_fetch_object ( $queryResult );
      
      if (($resource) && ($resource->hours != 0)){
        $link = "/timesheet/update/" . $resource->nid . '/' . $day;
        $rowdata[$project[title]] = '<a href="' . $link . '"><img class="edit-pencil" src="/' 
                                    . drupal_get_path('module', 'finanzen') . '/img/pencil.png" /></a>' 
                                    . '<span class="inserted-time">' . round($resource->hours, 2) . ' hours</span>' ;
      } 
      else {
        if ($resource->hours == 0) {
          $query_delete = "DELETE FROM finanzen_timesheet_worker_hours_projects WHERE nid='" . $resource->nid . "'";
          $query_deleteResult = db_query ( $query_delete );
        }
        $link = '/timesheet/insert/' . $project[nid] . '/' . $day . '/' . $user->uid ;
        $rowdata[$project[title]] = '<a href="' . $link . '"><img class="edit-pencil" src="/'
                                    . drupal_get_path('module', 'finanzen') . 
                                    '/img/pencil.png" /></a><span class="edit-empty"> Empty</span>';
      }
    }
    
    if (_finanzen_is_weekend($day))
      $row = array('data' => $rowdata, 'class' => "weekend");
    else
	  $row = array('data' => $rowdata);

    $rows[] = $row; 
  }
  
  $content .= theme_table($header, $rows, array('class' => 'finanzen'));
  
  return $content;
}

/* --------------------- Insert time - project --------------------------- */

function finanzen_timesheet_insert ( $project_id, $date, $user_id )
{
  $content .= _finanzen_timesheet_insert_update_navigator($date);
  $content .= drupal_get_form ( 'finanzen_timesheet_insert_form', $project_id, $date, $user_id  );
  return $content;
}


function finanzen_timesheet_insert_form ( $form, $project_id, $date, $user_id )
{
  $human_date = date('l, d.m.Y', $date);
  $project_code = _finanzen_get_project_code($project_id);
 
  $form = array (
    'insert_time' => array (
      '#type' => 'fieldset',
      '#title' => t( 'How many hours did you work in the project <b>' 
                  . $project_code . '</b> on <b>' . $human_date . '</b> ?' ),
      'hours' => array ( '#type' => 'textfield',
                         '#title' => 'Hours',
                         '#maxlength' => 2,
                         '#default_value' => 0,
                         '#required' => TRUE,
                         '#prefix' => '<div class="insert-time-hours-field">',
                         '#suffix' => '</div>',
                       ),
      'minutes' => array ( '#type' => 'textfield',
                           '#title' => 'Minutes',
                           '#maxlength' => 2,
                           '#default_value' => 0,
                           '#required' => TRUE,
                           '#prefix' => '<div class="insert-time-minutes-field">',
                           '#suffix' => '</div>',
                         ),
      'project_id' => array ( '#type' => 'hidden', "#default_value" => $project_id ),
      'date' => array ( '#type' => 'hidden', "#default_value" => $date ),
      'user_id' => array ( '#type' => 'hidden', "#default_value" => $user_id ),
      'submit' => array ( '#type' => 'submit',  '#value' => t('Save'), ),
    ),
  );
  
  $form['#redirect'] = 'timesheet/month/' . _finanzen_first_day_month($date);
  
  return $form;
}


function finanzen_timesheet_insert_form_submit ( $form, &$form_state )
{
  $user_id = $form_state['values']['user_id'];
  $date = $form_state['values']['date'];
  $hours = $form_state['values']['hours'];
  $minutes = $form_state['values']['minutes'];
  $project_id = $form_state['values']['project_id'];
  $hours_total = $hours + ((1/60) * $minutes);
  
  $query = "INSERT into {finanzen_timesheet_worker_hours_projects} ( uid, date, hours, project_id ) 
            values ( %d, '%s', %f, %d )";
  $queryResult = db_query ( $query, $user_id, $date, $hours_total, $project_id );
  
  if ($queryResult)
    drupal_set_message("Changes saved correctly", "status");
}


function finanzen_timesheet_insert_form_validate($form, &$form_state) {
  if (($form_state['values']['hours'] > 24) || !(is_numeric($form_state['values']['hours']))) {
    form_set_error('hours', t('Hours: You must enter a numeric value between 0 and 24.'));
  }
    if (($form_state['values']['minutes'] > 59) || !(is_numeric($form_state['values']['minutes']))) {
    form_set_error('minutes', t('Minutes: You must enter a numeric value between 0 and 59.'));
  }
}

/* --------------------- Update time - project --------------------------- */

function finanzen_timesheet_update ( $id, $date )
{
  $content .= _finanzen_timesheet_insert_update_navigator($date);
  $content .= drupal_get_form ( 'finanzen_timesheet_update_form', $id );
  return $content;
}


function finanzen_timesheet_update_form ( $form, $id )
{ 
  $query = "SELECT * FROM {finanzen_timesheet_worker_hours_projects} fuhp WHERE fuhp.nid=%d";
  $queryResult = db_query ( $query, $id );
  $resource = db_fetch_object ( $queryResult );
  
  $human_date = date('l, d.m.Y', $resource->date);
  $project_code = _finanzen_get_project_code($resource->project_id);

  $hours = intval($resource->hours);
  $minutes = round(($resource->hours - $hours) * 60, 0);
 
  $form = array (
    'update_time' => array (
      '#type' => 'fieldset',
      '#title' => t( 'How many hours did you work in the project <b>' . $project_code .
                     '</b> on <b>' . $human_date . '</b> ?' ),
      'hours' => array ( '#type' => 'textfield',
                         '#title' => 'Hours',
                         '#default_value' => $hours,
                         '#maxlength' => 2,
                         '#required' => TRUE,
                         '#prefix' => '<div class="insert-time-hours-field">',
                         '#suffix' => '</div>',
                       ),
      'minutes' => array ( '#type' => 'textfield',
                           '#title' => 'minutes',
                           '#default_value' => $minutes,
                           '#maxlength' => 2,
                           '#required' => TRUE,
                           '#prefix' => '<div class="insert-time-minutes-field">',
                           '#suffix' => '</div>',
                         ),
      'id' => array ( '#type' => 'hidden', "#default_value" => $id ),
      'submit' => array ( '#type' => 'submit',  '#value' => t('Save'), ),
    ),
  );
  
  $form['#redirect'] = 'timesheet/month/' . _finanzen_first_day_month($resource->date);

  return $form;
}


function finanzen_timesheet_update_form_submit ( $form, &$form_state )
{
  $id = $form_state['values']['id'];
  $hours = $form_state['values']['hours'];
  $minutes = $form_state['values']['minutes'];
  $hours_total = $hours + ((1/60) * $minutes);
  
  $query = "UPDATE finanzen_timesheet_worker_hours_projects SET hours='" . $hours_total . "' 
            WHERE nid='" . $id . "'";
  $queryResult = db_query ( $query );
  
  if ($queryResult)
    drupal_set_message("Changes saved correctly", "status");
}

function finanzen_timesheet_update_form_validate($form, &$form_state) {
  if (($form_state['values']['hours'] > 24) || !(is_numeric($form_state['values']['hours']))) {
    form_set_error('hours', t('Hours: You must enter a numeric value between 0 and 24.'));
  }
    if (($form_state['values']['minutes'] > 59) || !(is_numeric($form_state['values']['minutes']))) {
    form_set_error('minutes', t('Minutes: You must enter a numeric value between 0 and 59.'));
  }
}

/* --------------------- Edit month --------------------------------- */

function finanzen_timesheet_edit_month ( $first_day_month )
{
  $content .= _finanzen_timesheet_edit_month_navigator ($first_day_month);
  $content .= drupal_get_form ('finanzen_timesheet_edit_month_form', $first_day_month);
  return $content;
}


function finanzen_timesheet_edit_month_form ( $form, $first_day_month )
{
  // Clean the $form 
  $form = array ();
  
  global $user;
  
  $month = date('m', $first_day_month);
  $year = date('Y', $first_day_month);

  $all_projects = $user->og_groups;
  
  foreach ($all_projects as $project) {
    $project_node = node_load ($project[nid]);
  }
  
  $number_days = _finanzen_days_in_month($month, $year);
  
  for ($i = 1; $i <= $number_days; $i++) {

    $day = mktime(0,0,0,$month, $i, $year);
    
    foreach ($all_projects as $project) {  
      $query = "SELECT fuhp.nid, fuhp.hours FROM {finanzen_timesheet_worker_hours_projects} fuhp 
                WHERE fuhp.date=%d AND fuhp.uid=%d AND fuhp.project_id=%d" ;
      $queryResult = db_query ( $query, $day, $user->uid, $project[nid] );
      $resource = db_fetch_object ( $queryResult );
      
      if ($resource) {
        $hours = intval($resource->hours);
        $minutes = round(($resource->hours - $hours) * 60, 0);
        $css_class = "modified";
        $nid = $resource->nid;
      } 
      else {
        $hours = 0;
        $minutes = 0;
        $css_class = ""; 
        $nid = 0;
      }
      
      $project_code = _finanzen_get_project_code($project[nid]);
      
      $form['projects'][$day][$project[nid]] = array (
        'hours' => array ( '#type' => 'textfield',
                           '#title' => 'Hr',
                           '#default_value' => $hours,
                           '#maxlength' => 2,
                           '#weight' => 1,
                           '#required' => TRUE,
                           '#prefix' => '<div class="insert-time-hours-field ' . $css_class . '">',
                           '#suffix' => '</div>',
                         ),
        'minutes' => array ( '#type' => 'textfield',
                             '#title' => 'Min',
                             '#default_value' => $minutes,
                             '#maxlength' => 2,
                             '#weight' => 2,
                             '#required' => TRUE,
                             '#prefix' => '<div class="insert-time-minutes-field ' . $css_class . '">',
                             '#suffix' => '</div>',
                           ),
        'id' => array ( '#type' => 'hidden', "#default_value" => $nid ),
      );
    }
  }
  
  // Add submit button
  $form['submit'] = array (
      '#type' => 'submit',
      '#value' => t('Save'),
  );
  // Add theming function
  $form['#theme'] = 'finanzen_timesheet_edit_month_form_theme';
  // This line is needed in order to keep the tree structure in form_state
  $form['projects']['#tree'] = TRUE;
  // Add redirection
  $form['#redirect'] = 'timesheet/month/' . $first_day_month;
  
  return $form; 

}


function theme_finanzen_timesheet_edit_month_form_theme ( $form ) {

  $header = array('date' => array('data' => t("Date"),'class' => 'column_date'));
  
  $first_child = reset($form['projects']);
  
  $i=0;
  foreach (element_children($first_child) as $project_id) {
    $project_code = _finanzen_get_project_code($project_id);
    $header[$project_code] = array('data' => $project_code, 'class' => 'column_project');
    $projects[$i] = $project_id;
    $i++;
  }
  
  foreach (element_children($form['projects']) as $day) {
    $rowdata = array();
    $rowdata[] = date('d.m.Y, D', (float)$day);
    for ($j=0;$j<$i;$j++) {
	  $rowdata[] = drupal_render($form['projects'][$day][$projects[$j]]);
    }
      
    if (_finanzen_is_weekend($day))
      $row = array('data' => $rowdata, 'class' => "weekend");
    else
      $row = array('data' => $rowdata);
      
    $rows[] = $row;
  }
  
  // This is where we create the table using theme()
  $output .= theme('table', $header, $rows, array('class' => 'finanzen timesheet-edit-month'));
  
  $output .= drupal_render($form);
  return $output;
}


function finanzen_timesheet_edit_month_form_validate ( $form, &$form_state ) {
  
  foreach (element_children($form_state['values']['projects']) as $date) {
    
    foreach (element_children($form_state['values']['projects'][$date]) as $project_id) {
    
      if (($form_state['values']['projects'][$date][$project_id]['hours'] > 24) ||
         !(is_numeric($form_state['values']['projects'][$date][$project_id]['hours']))) {
        form_set_error("projects][$date][$project_id][hours", t('Hours: You must enter a numeric value between 0 and 24.'));
      }
      
      if (($form_state['values']['projects'][$date][$project_id]['minutes'] > 59) ||
         !(is_numeric($form_state['values']['projects'][$date][$project_id]['minutes']))) {
        form_set_error("projects][$date][$project_id][minutes", 
                       t('Hours: You must enter a numeric value between 0 and 59.'));
      }
    }
  }
}


function finanzen_timesheet_edit_month_form_submit ( $form, &$form_state )
{
  global $user;
  
  foreach (element_children($form_state['values']['projects']) as $date) {
    
    foreach (element_children($form_state['values']['projects'][$date]) as $project_id) {
        
      $hours = $form_state['values']['projects'][$date][$project_id]['hours'];
      $minutes = $form_state['values']['projects'][$date][$project_id]['minutes'];
      $hours_total = $hours + ((1/60) * $minutes);
        
      $reg_id = $form_state['values']['projects'][$date][$project_id]['id'];
      
      if ($reg_id != 0) { // DB Register already exists
	    $query = "UPDATE finanzen_timesheet_worker_hours_projects SET hours='" . $hours_total . "' 
                  WHERE nid='" . $reg_id . "'";
        $queryResult = db_query ( $query );   
      } 
      else { // DB Register does not exists
        if (($form_state['values']['projects'][$date][$project_id]['hours'] != 0) ||
            ($form_state['values']['projects'][$date][$project_id]['minutes']!= 0)) {
	      $query = "INSERT into {finanzen_timesheet_worker_hours_projects} ( uid, date, hours, project_id )
                    values ( %d, '%s', %f, %d )";
          $queryResult = db_query ( $query, $user->uid, $date, $hours_total, $project_id );   
        }
      }
    }
  }
  
  if ($queryResult)
      drupal_set_message("Changes saved correctly", "status");

}

