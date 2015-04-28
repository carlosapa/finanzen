<?php 

/**
 * This function outputs a timesheet month navigator
 */
function _finanzen_timesheet_month_navigator($date, $user)
{ 
  $current_date = time ();
  $current_first_day_month = _finanzen_first_day_month($current_date);
  $user_first_day_month = _finanzen_first_day_month($user->created);
  
  $content .= '<div class="navigator">';
  
  $content .= '<div class="navigator-back"><a href="/finanzen/reports/timesheet"> &#8592; zurück </a></div>';
  
  $content .= '<div class="navigator-title-klein black bold">' . _finanzen_get_user_name($user->uid) . '</div>';
  
  if (_finanzen_timesheet_is_month_closed($date, $user->uid)) {
  	$link_lock = '<a href="/finanzen/reports/timesheet/change_status/' . $date . '/' . $user->uid .
  	 	         '?' . drupal_get_destination() . '"><img class="locker" src="/'. drupal_get_path('module', 'finanzen') . 
                 '/img/lock-closed.png" /></a>';
    $class_lock = "closed";
  }
  else {
     $link_lock = '<a href="/finanzen/reports/timesheet/change_status/' . $date . '/' . $user->uid . 
                  '?' . drupal_get_destination() . '"><img class="locker" src="/'. drupal_get_path('module', 'finanzen') . 
                  '/img/lock-open.png" /></a>';
     $class_lock = "open";				
  }
  
  $content .= '<div style="float: left;" class="' . $class_lock . '">' . $link_lock . '</div>';
  
  $content .= '<div class="navigator-next">';
  if ($date < $current_first_day_month) {
	$content .= '<a title="Go to next month" href="/timesheet/month/' 
	            . _finanzen_get_first_day_next_month($date) . '/' . $user->uid . '"> &#8811; </a>';
  }
  else { $content .= '<span class="muted"> &#8811; </span>'; }
  $content .= "</div>";
  
  $content .= '<div class="navigator-date bold black">' . date('F Y', $date) . '</div>';
  
  $content .= '<div class="navigator-prev">';
  if ($date > $user_first_day_month) {
  	$content .= '<a title="Go to previous month" href="/timesheet/month/' 
  	            . _finanzen_get_first_day_previous_month($date) . '/' . $user->uid .'"> &#8810; </a>';
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
  $content .= '<div class="navigator-back"><a href="/timesheet/' . $date . '"> &#8592; zurück </a></div>';
  $content .= '<div class="navigator-date bold black">' . date('F Y', $date) . '</div>';
  $content .= '</div>';
  
  return $content;
}

/**
 * This function outputs a timesheet edit month navigator
 */
function _finanzen_timesheet_insert_update_navigator ( $date ) 
{  
  $first_day_month = _finanzen_first_day_month($date);
  
  $content .= '<div class="navigator">';
  $content .= '<div class="navigator-back"><a href="/timesheet/month/' . $first_day_month . '"> &#8592; zurück </a></div>';
  $content .= '</div>';
  
  return $content;
}

/**
 * This function outputs a Finanzen month navigator
 */
function _finanzen_timesheet_navigator($date, $user_name, $user_created)
{ 
  if ($date == 0) {
    $current_date = time ();
    $first_day_year = _finanzen_first_day_year($current_date);
  }
  else { $first_day_year = _finanzen_first_day_year($date); }

  $current_date = time ();
  $current_first_day_year = _finanzen_first_day_year($current_date);
  
  $content .= '<div class="navigator">';

  $content .= '<div class="navigator-title-klein bold black">' . $user_name . '</div>';
  
  $content .= '<div class="navigator-next">';
  if ($first_day_year < $current_first_day_year) {
	$content .= '<a title="Go to next year" href="/timesheet/' 
	            . _finanzen_get_first_day_next_year($first_day_year) . '"> &#8811; </a>';
  }
  else { $content .= '<span class="muted"> &#8811; </span>'; }
  $content .= "</div>";
  
  $content .= '<div class="navigator-date bold black">' . date('Y', $first_day_year) . '</div>';
  
  $content .= '<div class="navigator-prev">';
  if ($first_day_year > $user_created) {
  	$content .= '<a title="Go to previous year" href="/timesheet/' 
  	            . _finanzen_get_first_day_previous_year($first_day_year) . '"> &#8810; </a>';
  }
  else { $content .= '<span class="muted"> &#8810; </span>'; }
  $content .= "</div>";
  
  $content .= "</div>"; //class="navigator"
   
  return $content;     
}

function _finanzen_timesheet_summary ( $date, $user_id ) {

	$query = "SELECT ftwhp.hours FROM {finanzen_timesheet_worker_hours_projects} ftwhp 
              WHERE ftwhp.date>=" . _finanzen_first_day_year($date) . " 
		      AND ftwhp.date<" . _finanzen_get_first_day_next_year($date) . " 
		      AND ftwhp.uid=" . $user_id;
		      
    $queryResult = db_query ( $query );
    
    $total_hours = 0;
    while ($elem = db_fetch_object($queryResult)) {
	    $total_hours += $elem->hours;
    }
    
    $rowdata = array('concept' => array('data' => "Arbeitsstunde gesamt", 'class' => 'not-empty bold')) + 
		       array('total' => array('data' => $total_hours, 'class' => 'not-empty'));
    $rows[] = array('data' => $rowdata, 'class' => 'light-grey');
    
    /* Contabilizamos días de vacaciones y enfermedad sólo para empleados fijos y a partir de 2015 */
    if ((_finanzen_get_user_labour_agreement($user_id) == "Employee") && ($date >= 1420066800)) {
        
        $query = "SELECT field_year_vacation_days_value FROM content_type_profile
	              INNER JOIN node ON node.type='profile' AND node.uid=" . $user_id . "
	              WHERE node.nid = content_type_profile.nid";
	    $queryResult = db_query ($query);
	    $elem = db_fetch_object($queryResult);
	    
	    if ($elem->field_year_vacation_days_value == NULL)
	      $jahresurlaub = "unbekannt";
	    else
	      $jahresurlaub = $elem->field_year_vacation_days_value;

		$rowdata = array('concept' => array('data' => "Jahresurlaub", 'class' => 'not-empty bold')) + 
		           array('total' => array('data' => $jahresurlaub, 'class' => 'not-empty'));
		$rows[] = array('data' => $rowdata, 'class' => 'light-grey');
	
		$now = time();
		
		$query = "SELECT * FROM {finanzen_timesheet_worker_day} ftwd 
     	          WHERE ftwd.date>=" . _finanzen_first_day_year($date) . " 
     	          AND ftwd.date<" . $now . " 
     	          AND ftwd.uid=" . $user_id . " AND ftwd.daytype=1";

	    $queryResult = db_query ($query);
	    $beanspruchter_urlaub = db_affected_rows();
		
		$rowdata = array('concept' => array('data' => "Beanspruchter Urlaub", 'class' => 'not-empty bold')) + 
		           array('total' => array('data' => $beanspruchter_urlaub, 'class' => 'not-empty'));
		$rows[] = array('data' => $rowdata, 'class' => 'light-grey');
		
		$query = "SELECT * FROM {finanzen_timesheet_worker_day} ftwd 
     	          WHERE ftwd.date>=" . _finanzen_first_day_year($date) . " 
     	          AND ftwd.date<" . $now . " 
     	          AND ftwd.uid=" . $user_id . " AND ftwd.daytype=2";

	    $queryResult = db_query ($query);
	    $krankentage = db_affected_rows();
		
		$rowdata = array('concept' => array('data' => "Krankentage", 'class' => 'not-empty bold')) + 
		           array('total' => array('data' => $krankentage, 'class' => 'not-empty'));
		$rows[] = array('data' => $rowdata, 'class' => 'light-grey');
		
	
	}
	
	$content .= theme_table(array(), $rows, array('class' => 'finanzen-klein reports'));
	
	return $content;

}

/**
 * Mainpage for Timesheet feature
 */
function finanzen_timesheet( $date = 0 ) {

  if ($date == 0) {
    $current_date = time ();
    $first_day_year = _finanzen_first_day_year($current_date);
  }
  else 
    $first_day_year = _finanzen_first_day_year($date);
  
  global $user;
  
  if (empty($user->og_groups)) {
    $content .= 'You do not belong to any project. Go and <a href="og">join a project</a>.';
  }
  else {
  
    $content .= _finanzen_timesheet_summary($first_day_year, $user->uid);
    
    $first_month_user = _finanzen_first_day_month($user->created);
    $content .= _finanzen_timesheet_navigator($first_day_year, $user->name, $first_month_user);
  
    $all_projects = $user->og_groups;
    
    $months_curr_year = _finanzen_list_months_year ($first_day_year);
    
    /* Build header */
    $header['date'] = array('data' => "", 'class' => 'column_date');
    foreach ($all_projects as $project) {
      $project_code = _finanzen_get_project_code($project[nid]);
      $header[$project_code] = array('data' => $project_code, 'class' => 'column_project');
    }
    $header['total'] = array('data' => "Total", 'class' => 'column_total');
    $header['edit'] = t("Edit");
    
    /* Get data */
    foreach ($months_curr_year as $month_name => $month_data) {
    
      if (/*(time() >= $month_data['date']) &&*/ ($first_month_user <= $month_data['date'])) {
	      $total_hours_worker_month = 0;
	   	  
	   	  $rowdata['date'] = date('M Y', $month_data['date']);
	   	                       
	      foreach ($all_projects as $project) {
	          $total_hours_worker_month_project = 0;
		      $query = "SELECT ftwhp.nid, ftwhp.hours FROM {finanzen_timesheet_worker_hours_projects} ftwhp 
		                WHERE ftwhp.date>=" . $month_data['date'] . " 
		                AND ftwhp.date<" . _finanzen_get_first_day_next_month($month_data['date']) . " 
		                AND ftwhp.uid=" . $user->uid . " 
		                AND ftwhp.project_id=" . $project[nid];
		      $queryResult = db_query ( $query );
		      
		      while ($resource = db_fetch_object($queryResult)){
		        $total_hours_worker_month_project += $resource->hours;
		        $total_hours_worker_month += $resource->hours;
		      }
		      
		      $rowdata[$project[title]] =  round($total_hours_worker_month_project, 2) . ' hr' ;
	      }
	      
	      $rowdata['total'] = '<strong>' . round($total_hours_worker_month, 2) . ' hr</strong>';
	      $rowdata['edit'] = '<a href="/timesheet/edit-month/' . $month_data['date'] . '"><img class="edit-pencil" src="/'
	                           . drupal_get_path('module', 'finanzen') . '/img/pencil.png" /></a>';
	
		  $rows[] = $rowdata;
	  }
    }
 
    $content .= theme_table($header, $rows, array('class' => 'finanzen timesheet timesheet-main'));
  }
  
  return $content;
}

/**
 * Users Timesheet for the given month -> $first_day_month
 */
function finanzen_timesheet_month( $first_day_month, $user_id ) {

  global $user;
  $curr_user = $user;
  
  $user_timesheet = user_load($user_id);
  
  $month = date('m', $first_day_month);
  $year = date('Y', $first_day_month);
  
  
  if (($curr_user->uid != $user_id) ||  (in_array("administrator", $curr_user->roles)) || ($curr_user->uid == 1)){ 
  // Si el usuario dueño del timsheet no es el mismo que el que mira, el que mira es administrador o el que mira es Diego.
    $content .= _finanzen_timesheet_month_navigator($first_day_month, $user_timesheet);
  }
  else {
    $content .= '<div class="navigator"><div class="navigator-date bold black">' . date('F Y', $first_day_month) . '</div></div>';
  } 
  
  $number_days = _finanzen_days_in_month($month, $year);

  $all_projects = $user_timesheet->og_groups;
  
  $header = array('date' => array('data' => t("Date"),'class' => 'column_date'));
  
  foreach ($all_projects as $project) {
    $project_code = _finanzen_get_project_code($project[nid]);
    $header[$project_code] = array('data' => $project_code, 'class' => 'column_project');
  }
  
  $rows = array ();
  
  for ($i = 1; $i <= $number_days; $i++) {
    $day = mktime(0,0,0,$month, $i, $year);
    $rowdata = array ();
    //$rowdata['date'] = date('d.m.Y, D', $day);
    $rowdata['date'] = array ('data' => date('d.m.Y, D', $day), 'class' => 'column_date');
        
    foreach ($all_projects as $project) {
      $query = "SELECT ftwhp.nid, ftwhp.hours FROM {finanzen_timesheet_worker_hours_projects} ftwhp 
                WHERE ftwhp.date=%d AND ftwhp.uid=%d AND ftwhp.project_id=%d";
      $queryResult = db_query ( $query, $day, $user_id, $project[nid] );
      $resource = db_fetch_object ( $queryResult );
      
      if (($resource) && ($resource->hours != 0)){
        $link = "/timesheet/update/" . $resource->nid . '/' . $day;
        $rowdata[$project[title]] = '<span class="inserted-time">' . round($resource->hours, 2) . ' hr</span>' ;
      } 
      else {
        if ($resource->hours == 0) {
          $query_delete = "DELETE FROM finanzen_timesheet_worker_hours_projects WHERE nid='" . $resource->nid . "'";
          $query_deleteResult = db_query ( $query_delete );
        }
        $link = '/timesheet/insert/' . $project[nid] . '/' . $day . '/' . $user_id ;
        $rowdata[$project[title]] = '<span class="edit-empty">0 hr</span>';
      }
    }
    
    if (_finanzen_is_weekend($day) || _finanzen_is_feiertage($day))
      $row = array('data' => $rowdata, 'class' => "weekend");
    elseif (_finanzen_is_holyday_sikness($day, $user_id) == 1)
      $row = array('data' => $rowdata, 'class' => "holydays");
    elseif (_finanzen_is_holyday_sikness($day, $user_id) == 2)
      $row = array('data' => $rowdata, 'class' => "sickness");  
    else
      $row = array('data' => $rowdata);

    $rows[] = $row; 
  }
  
  $content .= theme_table($header, $rows, array('class' => 'finanzen timesheet timesheet-admin-view'));
  
  return $content;
}

/* --------------------- Insert time - project OBSOLETA --------------------------- */
/*
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
*/

/* --------------------- Update time - project OBSOLETA --------------------------- */
/*
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
*/
/* --------------------- Edit month --------------------------------- */

function finanzen_timesheet_edit_month ( $first_day_month )
{
  global $user;
  if (_finanzen_timesheet_is_month_closed($first_day_month, $user->uid)) {
    drupal_set_message("Month is closed, you cannot modify it. 
                        If you want to modify this month, contact an administrator", "warning");
    $content .= finanzen_timesheet_month( $first_day_month, $user->uid );
  }
  else {
    $content .= _finanzen_timesheet_edit_month_navigator ($first_day_month);
    $content .= drupal_get_form ('finanzen_timesheet_edit_month_form', $first_day_month);
  }
  
  return $content;
    
}


function finanzen_timesheet_edit_month_form ( $form, $first_day_month )
{
  // Clean the $form 
  $form = array ();
  
  global $user;
  
  $month = date('m', $first_day_month);
  $year = date('Y', $first_day_month);
  $daytype = array("Arbeitstag", "Urlaubstag", "Krankentag");

  $all_projects = $user->og_groups;
  
  foreach ($all_projects as $project) {
    $project_node = node_load ($project[nid]);
  }
  
  $number_days = _finanzen_days_in_month($month, $year);
  
  for ($i = 1; $i <= $number_days; $i++) {

    $day = mktime(0,0,0,$month, $i, $year);
    
    if ((!_finanzen_is_weekend($day)) && (!_finanzen_is_feiertage($day))) {
    
       $query = "SELECT ftwd.nid, ftwd.daytype FROM {finanzen_timesheet_worker_day} ftwd 
                 WHERE ftwd.date=" . $day . " AND ftwd.uid=" . $user->uid;
       $queryResult = db_query ( $query );
       if (!$resource = db_fetch_object ( $queryResult )) {
         $resource->daytype = 0;
         $resource->nid = 0;
       }
    
	   $form['daytype'][$day] = array (
	         'daytype' => array ( '#type' => 'select',
	                              '#options' => $daytype,
	                              '#default_value' => $resource->daytype,
	                              '#required' => TRUE,
	                              '#prefix' => '<div class="inflow-add to-left bleed-right">',
	                              '#suffix' => '</div>',
	                            ),
	         'id' => array ( '#type' => 'hidden', "#default_value" => $resource->nid ),                   
	    
	    );
	}
    
    foreach ($all_projects as $project) {  
      $query2 = "SELECT fuhp.nid, fuhp.hours FROM {finanzen_timesheet_worker_hours_projects} fuhp 
                WHERE fuhp.date=%d AND fuhp.uid=%d AND fuhp.project_id=%d" ;
      $queryResult2 = db_query ( $query2, $day, $user->uid, $project[nid] );
      $resource2 = db_fetch_object ( $queryResult2 );
      
      if ($resource2) {
        $hours = number_format($resource2->hours, 2, ".", ",");
        $css_class = "modified";
        $nid = $resource2->nid;
      } 
      else {
        $hours = 0;
        $css_class = ""; 
        $nid = 0;
      }
      
      $project_code = _finanzen_get_project_code($project[nid]);
      
      $form['projects'][$day][$project[nid]] = array (
        'hours' => array ( '#type' => 'textfield',
                           '#title' => 'Hr',
                           '#default_value' => $hours,
                           '#maxlength' => 5,
                           '#weight' => 1,
                           '#required' => TRUE,
                           '#prefix' => '<div class="insert-time-hours-field ' . $css_class . '">',
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
  $form['daytype']['#tree'] = TRUE;
  // Add redirection
  $form['#redirect'] = 'timesheet/edit-month/' . $first_day_month;
  
  return $form; 

}


function theme_finanzen_timesheet_edit_month_form_theme ( $form ) {

  global $user;

  $header['daytype'] = array('data' => t("Day Type"),'class' => 'column_daytype');
  $header['date'] = array('data' => t("Date"),'class' => 'column_date');
  
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
    
    if ((!_finanzen_is_weekend($day)) && (!_finanzen_is_feiertage($day))) {
    	$rowdata[] = drupal_render($form['daytype'][$day]);
    }
    elseif (_finanzen_is_weekend($day))
      $rowdata[] = "Wochenende";
    else
      $rowdata[] = "Feiertag";

   // $rowdata[] = date('d.m.Y, D', (float)$day);
    $rowdata[] = array ('data' => date('d.m.Y, D', (float)$day), 'class' => 'column_date');
    
    for ($j=0;$j<$i;$j++) {
	  $rowdata[] = drupal_render($form['projects'][$day][$projects[$j]]);
    }
      
    if (_finanzen_is_weekend($day) || _finanzen_is_feiertage($day))
      $row = array('data' => $rowdata, 'class' => "weekend");
    elseif (_finanzen_is_holyday_sikness($day, $user->uid) == 1)
      $row = array('data' => $rowdata, 'class' => "holydays");
    elseif (_finanzen_is_holyday_sikness($day, $user->uid) == 2)
      $row = array('data' => $rowdata, 'class' => "sickness");  
    else
      $row = array('data' => $rowdata);
      
    $rows[] = $row;
  }
  
  // This is where we create the table using theme()
  $output .= theme('table', $header, $rows, array('class' => 'finanzen timesheet timesheet-edit-month'));
  
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
    }
  }
}


function finanzen_timesheet_edit_month_form_submit ( $form, &$form_state )
{
  global $user;
  
  foreach (element_children($form_state['values']['daytype']) as $date) {
  
    $worker_day_id = $form_state['values']['daytype'][$date]['id'];
    $worker_day_daytype = $form_state['values']['daytype'][$date]['daytype'];
  
    if ($worker_day_id != 0) { // DB Register already exists
    	if ($worker_day_daytype != 0) {   
	    	$query = "UPDATE finanzen_timesheet_worker_day 
	    			  SET daytype=" . $worker_day_daytype . "
	                  WHERE nid=" . $worker_day_id ;
	        $queryResult = db_query ( $query );
	     }
	     else {
	     	$query_delete = "DELETE FROM finanzen_timesheet_worker_day WHERE nid=" . $worker_day_id;
	        $query_deleteResult = db_query ( $query_delete );
	     }
    }
    else { // DB Register does not exists
	    if ($worker_day_daytype != 0) {
	      $query = "INSERT into finanzen_timesheet_worker_day ( uid, date, daytype )
                    values ( %d, '%s', %d )";
          $queryResult = db_query ( $query, $user->uid, $date, $worker_day_daytype);   
        }
    }
  }
  
  foreach (element_children($form_state['values']['projects']) as $date) {
    
    foreach (element_children($form_state['values']['projects'][$date]) as $project_id) {
        
      $hours = $form_state['values']['projects'][$date][$project_id]['hours'];
      $hours_total = $hours;
        
      $reg_id = $form_state['values']['projects'][$date][$project_id]['id'];
      
      if ($reg_id != 0) { // DB Register already exists
	    $query = "UPDATE finanzen_timesheet_worker_hours_projects SET hours='" . $hours_total . "' 
                  WHERE nid='" . $reg_id . "'";
        $queryResult = db_query ( $query );   
      } 
      else { // DB Register does not exists
        if ( $form_state['values']['projects'][$date][$project_id]['hours'] != 0 ) {
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

