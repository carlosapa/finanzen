<?php 

/* ----------------------------------------------------------------------------- */
/* ---------------------------- OUTFLOW PERSONAL ------------------------------- */
/* ----------------------------------------------------------------------------- */

function finanzen_outflow_personal_general ( $date = 0 )
{
  if ($date == 0) {
    $current_date = time ();
    $first_day_month = _finanzen_first_day_month($current_date);
  }
  else { $first_day_month = $date; }
  
  $content .=  _finanzen_outflow_personal_general_navigator($first_day_month);
  $content .= finanzen_reports_outflow_personal($first_day_month);
  $content .= finanzen_reports_outflow_personal_extern($first_day_month);

  return $content;
}

function _finanzen_outflow_personal_general_navigator($date)
{ 
  $content .= '<div class="navigator">';
  $content .= '<div class="navigator-back"><a href="/finanzen/' . $date . '"> &#8592; zurück</a></div>';
  $content .= '<div class="navigator-date">Jahresübersicht Personalkosten + Projektbezogene Ausgaben ' 
              . date('Y', $date) . "</div>";
  $content .= '<div class="navigator-next">';
  
  if (time() > _finanzen_get_first_day_next_year($date)) {
    $content .= '<a title="Go to next year" href="/finanzen/outflow/personal/general/' 
	            . _finanzen_get_first_day_next_year($date) . '"> &#8811; </a>';
  }
  else 
    $content .= '<span class="muted"> &#8811; </span>';
    
  $content .= "</div>";
   
  $content .= '<div class="navigator-prev">';
  if ($date > ABSOLUTE_INITIAL_TIME) {
  	$content .= '<a title="Go to previous year" href="/finanzen/outflow/personal/general/' 
  	            . _finanzen_get_first_day_previous_year($date) . '"> &#8810; </a>';
  }
  else
    $content .= '<span class="muted"> &#8810; </span>';
  
  $content .= "</div>";
  
  $content .= "</div>"; // class="navigator"
   
  return $content;     
}


function finanzen_reports_outflow_personal( $date = 0 ) {
  
  if ($date == 0) {
    $current_date = time ();
    $first_day_month = _finanzen_first_day_month($current_date);
  }
  else { $first_day_month = $date; }
  
  $row_personalkosten_gesamt = array();
  $row_personalkosten_gesamt['person'] = "Gesamt Personalkosten";
  $row_freelancers_gesamt = array();
  $row_freelancers_gesamt['person'] = "Gesamt Freelancers";
  $row_angestellete_gesamt = array();
  $row_angestellete_gesamt['person'] = "Gesamt Angestellete";  
  $total_all_users_year = 0;
  
  $months_curr_year = _finanzen_list_months_year ($first_day_month, 1);
  $header = array('person' => array('data' => t("Eigeneleistung"),'class' => 'bold black column_person')) + 
  			$months_curr_year + array('total' => array('data' => t("Gesamt"),'class' => 'column_total'));
                    
  /* $query = "SELECT DISTINCT finanzen_outflow_worker_month.uid FROM finanzen_outflow_worker_month 
               INNER JOIN users ON finanzen_outflow_worker_month.uid=users.uid
               LEFT JOIN users_roles ON finanzen_outflow_worker_month.uid=users_roles.uid
               WHERE date>=" . _finanzen_first_day_year($first_day_month) . "
               AND date<" . _finanzen_get_first_day_next_year($first_day_month) . "
               ORDER BY users_roles.rid DESC, users.name"; */
            
  $query = "SELECT DISTINCT finanzen_outflow_worker_month.uid FROM finanzen_outflow_worker_month 
            INNER JOIN node ON node.type='profile' AND node.uid = finanzen_outflow_worker_month.uid
  	  		INNER JOIN content_type_profile ON content_type_profile.nid = node.nid
            INNER JOIN users ON finanzen_outflow_worker_month.uid=users.uid
            LEFT JOIN users_roles ON finanzen_outflow_worker_month.uid=users_roles.uid
            WHERE date>=" . _finanzen_first_day_year($first_day_month) . " 
            AND date<" . _finanzen_get_first_day_next_year($first_day_month) . " 
            ORDER BY content_type_profile.field_labour_agreement_value DESC,users_roles.rid DESC, users.name";          
  $queryResult = db_query ($query);
  
  $first_fest = TRUE;
 
  while ($user = db_fetch_object($queryResult)){
   
    $row = array('person' => array('data' => _finanzen_get_user_name($user->uid), 'class' => 'not-empty'));
    $total_user_year = 0;
    $user_agreement = _finanzen_get_user_labour_agreement($user->uid);
    
	foreach ($months_curr_year as $month_name => $month_data ) {
	
		$total_user_month = 0;
		if (empty($row_personalkosten_gesamt[$month_name])): $row_personalkosten_gesamt[$month_name] = 0; endif;
		if (empty($row_freelancers_gesamt[$month_name])): $row_freelancers_gesamt[$month_name] = 0; endif;
		if (empty($row_angestellete_gesamt[$month_name])): $row_angestellete_gesamt[$month_name] = 0; endif;
		
		$query2 = "SELECT * from finanzen_outflow_worker_month
				   WHERE uid=" . $user->uid . " AND date=" . $month_data['date'];
		$queryResult2 = db_query ( $query2 );
		
		while ($outflow = db_fetch_object($queryResult2)) {
			$total_user_month += $outflow->spend;
			$row_personalkosten_gesamt[$month_name] += $outflow->spend;
			$total_user_year += $outflow->spend;
			if ($user_agreement == "Freelancer")
			  $row_freelancers_gesamt[$month_name] += $outflow->spend;
			else //$user_agreement == "Employee"
			  $row_angestellete_gesamt[$month_name] += $outflow->spend;
		}
		
		if ($total_user_month > 0)
  		  $row[] = array('data' => _finanzen_format_number($total_user_month), 'class' => 'not-empty');
  		else 
    	  $row[] = _finanzen_format_number($total_user_month);

	}
	
	if ($first_fest && ($user_agreement == "Employee")) {	
		$first_fest = FALSE;
		$total_year_freelancers = 0;
	    foreach ($row_freelancers_gesamt as $month_name => $month_data) {
		   if ($month_name != "person") {
		      $total_year_freelancers += $month_data;
			  $row_freelancers_gesamt[$month_name] = _finanzen_format_number($month_data);
		   }
	    }
	    $row_freelancers_gesamt['total'] = _finanzen_format_number($total_year_freelancers);
	    $rows[] = array('data' => $row_freelancers_gesamt, 'class' => 'not-empty bold light-grey');
	}
	
	
	$total_all_users_year += $total_user_year;
	
	$row['total'] = array('data' => _finanzen_format_number($total_user_year), 'class' => 'not-empty bold');
	$rows[] = $row;
  }
  
  /*
  if ($row_freelancers_gesamt) {
      $total_year_freelancers = 0;
	  foreach ($row_freelancers_gesamt as $month_name => $month_data) {
		 if ($month_name != "person") {
		    $total_year_freelancers += $month_data;
			$row_freelancers_gesamt[$month_name] = _finanzen_format_number($month_data);
		 }
	  }
	  $row_freelancers_gesamt['total'] = _finanzen_format_number($total_year_freelancers);
  }
  $rows[] = array('data' => $row_freelancers_gesamt, 'class' => 'not-empty bold light-grey');
  */

  if ($row_angestellete_gesamt) {
      $total_year_angestellte = 0;  
	  foreach ($row_angestellete_gesamt as $month_name => $month_data) {
		 if ($month_name != "person") {
			 $total_year_angestellte += $month_data;
			 $row_angestellete_gesamt[$month_name] = _finanzen_format_number($month_data);
		 }
	  }
	  $row_angestellete_gesamt['total'] = _finanzen_format_number($total_year_angestellte);
  }
  $rows[] = array('data' => $row_angestellete_gesamt, 'class' => 'not-empty bold light-grey');

  if ($row_personalkosten_gesamt) {
	  foreach ($row_personalkosten_gesamt as $month_name => $month_data) {
		 if ($month_name != "person")
			$row_personalkosten_gesamt[$month_name] = _finanzen_format_number($month_data);
	  }
	  $row_personalkosten_gesamt['total'] = _finanzen_format_number($total_all_users_year);
  }
  $rows[] = array('data' => $row_personalkosten_gesamt, 'class' => 'not-empty bold grey');
  
  $content .= theme_table($header, $rows, array('class' => 'finanzen reports'));
  
  return $content;
}


function finanzen_reports_outflow_personal_extern( $date = 0 ) {
  
  if ($date == 0) {
    $current_date = time ();
    $first_day_month = _finanzen_first_day_month($current_date);
  }
  else { $first_day_month = $date; }
  
  $last_row = array();
  $total_all_users_year = 0;
  
  $months_curr_year = _finanzen_list_months_year ($first_day_month, 4); 
  $header = array('name' => array('data' => t("Fremdeleistung"),'class' => 'bold black column_person')) + 
            $months_curr_year +
            array('total' => array('data' => t("Gesamt"),'class' => 'column_total'));
                    
  $query = "SELECT DISTINCT extern_id FROM finanzen_outflow_worker_extern_month 
  			WHERE date>=" . _finanzen_first_day_year($first_day_month) .
		  	" AND date<" . _finanzen_get_first_day_next_year($first_day_month) .
  			" ORDER BY extern_id";
  $queryResult = db_query ( $query );
  
  while ($extern = db_fetch_object($queryResult)){
    $row = array('name' => array('data' => _finanzen_outflow_personal_extern_get_extern_name($extern->extern_id),  
                                 'class' => 'not-empty'));
    $last_row['name'] = "Gesamt Beratungen";
    
    $total_user_year = 0;
	
	foreach ($months_curr_year as $month_name => $month_data ) {
		
		$total_extern_month = 0;
		if (empty($last_row[$month_name])): $last_row[$month_name] = 0; endif;
		
		$query2 = "SELECT * from finanzen_outflow_worker_extern_month
				   WHERE extern_id=" . $extern->extern_id . " AND date=" . $month_data['date'];
		$queryResult2 = db_query ( $query2 );
		
		while ($outflow = db_fetch_object($queryResult2)) {
			$total_extern_month += $outflow->spend;
			$last_row[$month_name] += $outflow->spend;
			$total_user_year += $outflow->spend;
		}
		
		if ($total_extern_month > 0)
  		  $row[] = array('data' => _finanzen_format_number($total_extern_month), 'class' => 'not-empty');
  		else
		  $row[] = _finanzen_format_number($total_extern_month);
	}
	
    $total_all_users_year += $total_user_year;
		
    $row['total'] = array('data' => _finanzen_format_number($total_user_year), 'class' => 'bold not-empty');
	$rows[] = $row;
  }

  if ($last_row) {
	  foreach ($last_row as $month_name => $month_data){
	       if ($month_name != "name")
			 $last_row[$month_name] = _finanzen_format_number($month_data);
	  }
	  $last_row['total'] = _finanzen_format_number($total_all_users_year);
  }
  
  $rows[] = array('data' => $last_row, 'class' => 'bold not-empty light-grey');
    
  $content .= theme_table($header, $rows, array('class' => 'finanzen reports'));

  return $content;
}


/* --------------------- Finanzen Personal Outflow Edit month --------------------------------- */

/**
 * Navigator
 */
function _finanzen_outflow_personal_edit_month_navigator ( $date ) 
{
  $content .= '<div class="navigator">';
  $content .= '<div class="navigator-back"><a href="/finanzen/outflow/personal/general/' . $date . '"> &#8592; zurück</a></div>';
  $content .= '<div class="navigator-date">' . date('F Y', $date) . '</div>';
  $content .= '</div>';
  
  return $content;
}


function finanzen_outflow_personal_edit_month ( $first_day_month )
{
  $content .= _finanzen_outflow_personal_edit_month_navigator ($first_day_month);
  $content .= drupal_get_form ('finanzen_outflow_personal_edit_month_form', $first_day_month);
  return $content;
}


function finanzen_outflow_personal_edit_month_form ( $form, $first_day_month )
{
  // Clean the $form 
  $form = array ();
  
  $query = "SELECT users.uid, users.name FROM users
  			LEFT JOIN users_roles ON users.uid=users_roles.uid
  			WHERE users.uid!=0 AND users.uid!=1
  			ORDER BY users_roles.rid DESC, users.name";
  $queryResult = db_query ($query);
    
  while ($current_user = db_fetch_object($queryResult)) {
    $query2 = "SELECT nid, spend, taxrate FROM finanzen_outflow_worker_month 
    		   WHERE uid=" . $current_user->uid . " AND date=" . $first_day_month;
    $queryResult2 = db_query ($query2);
    $resource2 = db_fetch_object ($queryResult2);
    
    if ($resource2) {
	    $css_class = "modified";	    
    }
    else {
	    $css_class = "";
	    $resource2->spend = 0;
	    $resource2->taxrate = 0;
    }
      	
  	$form[$first_day_month][$current_user->uid] = array (
      'spend' => array ( '#type' => 'textfield',
                         '#default_value' => $resource2->spend,
                         '#weight' => 1,
                         '#required' => TRUE,
                         '#prefix' => '<div class="outflow-personal-spend-field ' . $css_class . '">',
                         '#suffix' => '€</div>',       
                       ),
      'taxrate' => array ( '#type' => 'select',
                           '#default_value' => $resource2->taxrate,
                           '#options' => array('0' => '0%', '7' => '7%', '19' => '19%'),
                           '#weight' => 2,
                           '#required' => TRUE,
                         ),
      'id' => array ( '#type' => 'hidden' , "#default_value" => $resource2->nid ),
    );
  }
  
  // Add submit button
  $form['submit'] = array (
      '#type' => 'submit',
      '#value' => t('Save'),
  );
  // Add theming function
  $form['#theme'] = 'finanzen_outflow_personal_edit_month_form_theme';
  // This line is needed in order to keep the tree structure in form_state
  $form['#tree'] = TRUE;
  // Add redirection
  $form['#redirect'] = 'finanzen/' . $first_day_month;
  
  return $form; 
}


function theme_finanzen_outflow_personal_edit_month_form_theme ( $form ) {
     
  $header = array('name' => t("Name"), 'spend' => t("Spend"), 'taxrate' => t("Tax Rate"));
  
  //Get key value for the first array element
  $first_day_month = current(array_keys($form));
  
  $first_child = reset($form);
  
  foreach (element_children($first_child) as $user_id) {
    $rowdata = array();
    $rowdata[] = user_load($user_id)->name;
    $rowdata[] = drupal_render($form[$first_day_month][$user_id]['spend']);
    $rowdata[] = drupal_render($form[$first_day_month][$user_id]['taxrate']);
    $row = array('data' => $rowdata);
    $rows[] = $row;
  }
  
  // This is where we create the table using theme()
  $output .= theme('table', $header, $rows, array('class' => 'finanzen'));
  
  $output .= drupal_render($form);
  return $output;
}


function finanzen_outflow_personal_edit_month_form_validate ( $form, &$form_state ) {
  
  $first_day_month = current(array_keys($form));

  foreach (element_children($form_state['values'][$first_day_month]) as $user_id) {
  
    if (!(is_numeric($form_state['values'][$first_day_month][$user_id]['spend']))) {
      form_set_error("$first_day_month][$user_id][spend", t('Spend: You must enter a numeric value.'));
    }
  }
}


function finanzen_outflow_personal_edit_month_form_submit ( $form, &$form_state )
{
  $first_day_month = current(array_keys($form));
  
  foreach (element_children($form_state[$first_day_month]) as $user) {
  	$spend = $form_state['values'][$first_day_month][$user]['spend'];
  	$taxrate = $form_state['values'][$first_day_month][$user]['taxrate'];
  	$reg_id = $form_state['values'][$first_day_month][$user]['id'];
  	
  	if ($reg_id) { // DB Register already exists
  		if ($spend == 0) {
		    $query_delete = "DELETE FROM finanzen_outflow_worker_month WHERE nid='" . $reg_id . "'";
		    $queryResult = db_query ( $query_delete );	   		
	    }
	    else { // $spend != 0
		    $query = "UPDATE finanzen_outflow_worker_month SET spend=" . $spend . ", taxrate=" . $taxrate . 
	                 " WHERE nid='" . $reg_id . "'";
	        $queryResult = db_query ( $query );
	    }
    } 
    else { // DB Register does not exists
      if (($form_state['values'][$first_day_month][$user]['spend']!= 0) ||
          ($form_state['values'][$first_day_month][$user]['taxrate']!= 0)) {
	    $query = "INSERT into {finanzen_outflow_worker_month} ( uid, date, spend, taxrate )
                  values ( %d, '%s', %f, %d )";
        $queryResult = db_query ( $query, $user, $first_day_month, $spend, $taxrate );   
      }
    }
  }
  
  if ($queryResult)
      drupal_set_message("Changes saved correctly", "status");
}

/* ----------------------------------------------------------------------------- */
/* ------------------------ OUTFLOW PERSONAL EXTERN ---------------------------- */
/* ----------------------------------------------------------------------------- */

function finanzen_outflow_personal_extern_list () {
  
  $header = array('name' => t("Name"), 'change_status' => t("Change status"));
  
  $query = "SELECT * FROM finanzen_outflow_worker_extern ORDER BY active DESC, name";
  $queryResult = db_query ($query);
    
  while ($extern = db_fetch_object($queryResult)) {
    if ($extern->active == 1) {
	    $status_to_reach = 'deactivate';
	    $id_status_to_reach = '0';
    }
    else {
	    $status_to_reach = 'activate';
	    $id_status_to_reach = '1';
    }
  	$rowdata = array( 'name' => $extern->name,
  		    	      'change_status' => '<a href="/finanzen/outflow/personal/extern/change_status/' 
  	                                 . $extern->nid . '/' . $id_status_to_reach . '">' . $status_to_reach . '</a>' );
    
    if ($extern->active == 1)
      $row = array('data' => $rowdata);
    else
	  $row = array('data' => $rowdata, 'class' => "deactivated");
  	
  	$rows[] = $row;
  }

  $content .= theme_table($header, $rows , array('class' => 'finanzen outflow-personal-extern-list'));
  
  $content .= drupal_get_form ( 'finanzen_outflow_personal_extern_list_add_form' );
    
  return $content;
}


function finanzen_outflow_personal_extern_list_add_form ( $form )
{
  $form = array (
    'name' => array (
      '#type' => 'fieldset',
      '#title' => t( 'Add new extern' ),
      'name' => array ( '#type' => 'textfield',
        				'#title' => 'Name extern worker',
                        '#required' => TRUE,
                        '#prefix' => '<div class="to-left">',
                        '#suffix' => '</div>',),
      'submit' => array ( '#type' => 'submit',  
      					  '#value' => t('Save'),
      					  '#prefix' => '<div class="clear-left bleed-left">',
                          '#suffix' => '</div>',),
    ),
  );
  
  $form['#redirect'] = 'finanzen/outflow/personal/extern/list';
  
  return $form;
}


function finanzen_outflow_personal_extern_list_add_form_submit ( $form, &$form_state )
{
  $name = $form_state['values']['name'];
  $query = "INSERT into {finanzen_outflow_worker_extern} ( name ) values ('%s')";
  $queryResult = db_query ( $query, $name );
  if ($queryResult)
    drupal_set_message("Extern " . $name . " added correctly", "status");
}


function finanzen_outflow_personal_extern_change_status ( $id, $status ) 
{
  $query = "UPDATE finanzen_outflow_worker_extern SET active='" . $status . "' 
            WHERE nid='" . $id . "'";
  
  if ($status == 1)
    $message = "Extern activated";
  else
    $message = "Extern deactivated";
    
  if ($queryResult = db_query ( $query ))
    drupal_set_message($message, 'status');
  else
	drupal_set_message("Extern worker status not changed, an error occurred.", 'error');
	
  drupal_goto('finanzen/outflow/personal/extern/list');
}


/* --------------------- Finanzen personal extern Outflow month summary --------------------------------- */

/**
 * Navigator
 */
function _finanzen_outflow_personal_extern_navigator ( $date ) 
{
  $content .= '<div class="navigator">';
  $content .= '<div class="navigator-back"><a href="/finanzen/outflow/personal/general/' . $date . '"> &#8592; zurück</a></div>';
  $content .= '<div class="navigator-date">' . date('F Y', $date) . '</div>';
  $content .= '</div>';
  
  return $content;
}

function finanzen_outflow_personal_extern ( $date = 0 )
{
  if ($date == 0) {
    $current_date = time ();
    $first_day_month = _finanzen_first_day_month($current_date);
  }
  else { $first_day_month = $date; }
  
  $content .= '<b><a style="float:right; margin-right: 50px;" href="/finanzen/outflow/personal/extern/list">
  			   Administer Outflow Personal Extern</a></b>';

  
  $content .= _finanzen_outflow_personal_extern_navigator($first_day_month);
  
  $header = array('extern_name' => t("Extern Name"), 
  				  'project' => t("Project"), 
  				  'spend' => t("Spend"), 
  				  'taxrate' => t("Tax Rate"),
  				  'edit' => t("Edit"), 
  				  'delete' => t("Delete"));
  
  $query = "SELECT * FROM finanzen_outflow_worker_extern_month WHERE date=" . $first_day_month;
  $queryResult = db_query ($query);

  $total_amount = 0;
  while ($current_invoice = db_fetch_object($queryResult)) {
    $row = array( 'extern_name' => _finanzen_get_extern_name ($current_invoice->extern_id),
	   		      'project' => _finanzen_get_project_code($current_invoice->project_id),
	 			  'spend' => $current_invoice->spend . ' €',
 				  'taxrate' => $current_invoice->taxrate . '%',
 				  'edit' => '<a href="/finanzen/outflow/personal/extern/edit/' . 
   				  			   $current_invoice->nid . '/' . $first_day_month . '">edit</a>',
   				  'delete' => '<a href="/finanzen/outflow/personal/extern/delete/confirmation/' . 
   				  			   $current_invoice->nid . '/' . $first_day_month . '">delete</a>' );
	   				  
	$rows[] = $row;
	$total_amount += $current_invoice->spend;
  }
  
  $content .= theme_table($header, $rows , array('class' => 'finanzen outflow-personal-extern'));
  if (!$row)
   	$content .= "<p syle='margin-bottom: 20px;'>Still no invoices this month</p>";
  
  $content .= '<p><b>Total outflow personal extern ' . date('F Y', $first_day_month) . '</b>: ' . 
              _finanzen_format_number($total_amount) . '</p>';
  
  $content .= finanzen_outflow_personal_extern_add($first_day_month);
  
  return $content;
}

function finanzen_outflow_personal_extern_add ( $date )
{
  $query = "SELECT og.nid FROM og INNER JOIN node ON og.nid = node.nid"; // WHERE node.status=1";
  $queryResult = db_query ($query);
  while ($project_id = db_fetch_object($queryResult))
  	$projects[$project_id->nid] = _finanzen_get_project_code($project_id->nid);

  $query = "SELECT * FROM finanzen_outflow_worker_extern WHERE active=1";
  $queryResult = db_query ($query);
  while ($extern = db_fetch_object($queryResult))
  	$externs[$extern->nid] = $extern->name;
  	
  if (empty($externs)) {
	$content .=  "You cannot add external personal invoices before to load or activate external personal in the system. 
	              Do it <a href='/finanzen/outflow/personal/extern/list'>here</a>.";
  }
  else {
	  $content .= drupal_get_form ( 'finanzen_outflow_personal_extern_add_form', $date, $projects, $externs );
  }
  
  return $content;
}


function finanzen_outflow_personal_extern_add_form ( $form, $date, $projects, $externs )
{
  $human_date = date('F Y', $date);

  $form = array (
	 'outflow_extern_add' => array (
	   '#type' => 'fieldset',
	   '#title' => t( 'Add new invoice for <b>' . $human_date . '</b>' ),
	   'extern_id' => array ( '#type' => 'select',
	                          '#title' => 'Extern Name',
	                          '#options' => $externs,
	                          '#required' => TRUE,
	                          '#prefix' => '<div class="outflow-personal-extern-add to-left bleed-right">',
	                          '#suffix' => '</div>',
	                           ),        
	   'project' => array ( '#type' => 'select',
	                        '#title' => 'Project',
	                        '#options' => $projects,
	                        '#required' => TRUE,
	                        '#prefix' => '<div class="outflow-personal-extern-add to-left bleed-right">',
	                        '#suffix' => '</div>',
	                      ),
	   'spend' => array ( '#type' => 'textfield',
	                      '#title' => 'Spend in €',
	                      '#required' => TRUE,
	                      '#prefix' => '<div class="outflow-personal-extern-add to-left bleed-right">',
	                      '#suffix' => '</div>',
	                    ),
	   'taxrate' => array ( '#type' => 'select',
	                        '#title' => 'Tax rate',
	                        '#options' => array('0' => '0%', '7' => '7%', '19' => '19%'),
	                        '#required' => TRUE,
	                        '#prefix' => '<div class="outflow-personal-extern-add to-left ">',
	                        '#suffix' => '</div>',
	                      ),
	   'date' => array ( '#type' => 'hidden', "#default_value" => $date ),
	   'submit' => array ( '#type' => 'submit',  
	   					   '#value' => t('Save'), 
	      				   '#prefix' => '<div class="clear-left bleed-left">',
	                       '#suffix' => '</div>',),
	    ),
  );
	  
  $form['#redirect'] = 'finanzen/outflow/personal/extern/' . _finanzen_first_day_month($date);
	  
  return $form;
  
}

function finanzen_outflow_personal_extern_add_form_submit ( $form, &$form_state )
{
  $extern_id = $form_state['values']['extern_id'];
  $project_id = $form_state['values']['project'];
  $spend = $form_state['values']['spend'];
  $taxrate = $form_state['values']['taxrate'];
  $date = $form_state['values']['date'];
  
  $query = "INSERT into finanzen_outflow_worker_extern_month ( extern_id, project_id, spend, taxrate, date )
            values ( '%s', %d, %f, %d, '%s' )";
  $queryResult = db_query ( $query, $extern_id, $project_id, $spend, $taxrate, $date );
}


function finanzen_outflow_personal_extern_add_form_validate($form, &$form_state) 
{
  if (!is_numeric($form_state['values']['spend'])) {
    form_set_error("spend", t('Spend: You must enter a numeric value.'));
  }
}

/* ----------------------------------------------------------------------------- */
/* ---------------------------- OUTFLOW SONSTIGES ------------------------------ */
/* ----------------------------------------------------------------------------- */

function finanzen_outflow_sonstiges_general ( $date = 0 )
{
  if ($date == 0) {
    $current_date = time ();
    $first_day_month = _finanzen_first_day_month($current_date);
  }
  else { $first_day_month = $date; }
  
  $content .= _finanzen_outflow_sonstiges_general_navigator($first_day_month); 
  $content .= finanzen_reports_outflow_sonstiges($first_day_month);

  return $content;
}

function _finanzen_outflow_sonstiges_general_navigator($date)
{ 
  $content .= '<div class="navigator">';
  $content .= '<div class="navigator-back"><a href="/finanzen/' . $date . '"> &#8592; zurück</a></div>';
  $content .= '<div class="navigator-date">Jahresübersicht Bürokosten ' 
              . date('Y', $date) . "</div>";
  $content .= '<div class="navigator-next">';
  
  if (time() > _finanzen_get_first_day_next_year($date)) {
    $content .= '<a title="Go to next year" href="/finanzen/outflow/sonstiges/general/' 
	            . _finanzen_get_first_day_next_year($date) . '"> &#8811; </a>';
  }
  else 
    $content .= '<span class="muted"> &#8811; </span>';
    
  $content .= "</div>";
   
  $content .= '<div class="navigator-prev">';
  if ($date > ABSOLUTE_INITIAL_TIME) {
  	$content .= '<a title="Go to previous year" href="/finanzen/outflow/sonstiges/general/' 
  	            . _finanzen_get_first_day_previous_year($date) . '"> &#8810; </a>';
  }
  else
    $content .= '<span class="muted"> &#8810; </span>';
  
  $content .= "</div>";
  
  $content .= "</div>"; // class="navigator"
   
  return $content;     
}



function finanzen_outflow_sonstiges_categories () {
  
  $header = array('name' => t("Name"), 'change_status' => t("Change status"));
  
  $query = "SELECT * FROM finanzen_outflow_sonstiges_categories ORDER BY active DESC, name";
  $queryResult = db_query ($query);
    
  while ($category = db_fetch_object($queryResult)) {
    if ($category->active == 1) {
	    $status_to_reach = 'deactivate';
	    $id_status_to_reach = '0';
    }
    else {
	    $status_to_reach = 'activate';
	    $id_status_to_reach = '1';
    }
  	$rowdata = array( 'name' => $category->name,
  		    	      'change_status' => '<a href="/finanzen/outflow/sonstiges/categories/change_status/' 
  	                                 . $category->nid . '/' . $id_status_to_reach . '">' . $status_to_reach . '</a>' );
    
    if ($category->active == 1)
      $row = array('data' => $rowdata);
    else
	  $row = array('data' => $rowdata, 'class' => "deactivated");
  	
  	$rows[] = $row;
  }

  $content .= theme_table($header, $rows , array('class' => 'finanzen outflow-sonstiges-categories'));
  
  $content .= drupal_get_form ( 'finanzen_outflow_sonstiges_categories_add_form' );
    
  return $content;
}

function finanzen_outflow_sonstiges_categories_add_form ( $form )
{
  $form = array (
    'name' => array (
      '#type' => 'fieldset',
      '#title' => t( 'Add new category' ),
      'name' => array ( '#type' => 'textfield',
        				'#title' => 'Description category',
                        '#required' => TRUE,
                        '#prefix' => '<div class="to-left">',
                        '#suffix' => '</div>',),
      'submit' => array ( '#type' => 'submit',  
      					  '#value' => t('Save'),
      					  '#prefix' => '<div class="clear-left bleed-left">',
                          '#suffix' => '</div>',),
    ),
  );
  
  $form['#redirect'] = 'finanzen/outflow/sonstiges/categories';
  
  return $form;
}

function finanzen_outflow_sonstiges_categories_add_form_submit ( $form, &$form_state )
{
  $name = $form_state['values']['name'];
  $query = "INSERT into {finanzen_outflow_sonstiges_categories} ( name ) values ('%s')";
  $queryResult = db_query ( $query, $name );
  if ($queryResult)
    drupal_set_message("Category " . $name . " added correctly", "status");
}

function finanzen_outflow_sonstiges_categories_change_status ( $id, $status ) 
{
  $query = "UPDATE finanzen_outflow_sonstiges_categories SET active='" . $status . "' 
            WHERE nid='" . $id . "'";
  
  if ($status == 1)
    $message = "Category activated";
  else
    $message = "Category deactivated";
    
  if ($queryResult = db_query ( $query ))
    drupal_set_message($message, 'status');
  else
	drupal_set_message("Category status not changed, an error occurred.", 'error');
	
  drupal_goto('finanzen/outflow/sonstiges/categories');
}


/* --------------------- Finanzen sonstiges Outflow Edit month --------------------------------- */

/**
 * Navigator
 */
function _finanzen_outflow_sonstiges_edit_month_navigator ( $date ) 
{
  $content .= '<div class="navigator">';
  //$content .= '<div class="navigator-back"><a href="/finanzen/outflow/sonstiges/' . $date . '"> &#8592; zurück</a></div>';
  $content .= '<div class="navigator-back"><a href="/finanzen/outflow/sonstiges/general/' . $date . '"> &#8592; zurück</a></div>';
  $content .= '<div class="navigator-date">' . date('F Y', $date) . '</div>';
  $content .= '</div>';
  
  return $content;
}


function finanzen_outflow_sonstiges_edit_month ( $first_day_month )
{
  $content .= '<b><a style="float:right; margin-right: 50px;" href="/finanzen/outflow/sonstiges/categories">Administer Outflow sontsiges categories</a></b>';
  $content .= _finanzen_outflow_sonstiges_edit_month_navigator ($first_day_month);
  $content .= drupal_get_form ('finanzen_outflow_sonstiges_edit_month_form', $first_day_month);
  return $content;
}


function finanzen_outflow_sonstiges_edit_month_form ( $form, $first_day_month )
{
  // Clean the $form 
  $form = array ();
  
  $query = "SELECT * FROM finanzen_outflow_sonstiges_categories WHERE active=1";
  $queryResult = db_query ($query);
    
  while ($current_category = db_fetch_object($queryResult)) {
    $query2 = "SELECT spend, nid FROM finanzen_outflow_sonstiges_month 
    		   WHERE category_id=" . $current_category->nid . " AND date=" . $first_day_month;
    $queryResult2 = db_query ($query2);
    $resource2 = db_fetch_object ($queryResult2);
    
    if ($resource2)
	    $css_class = "modified";	    
    else {
	    $css_class = "";
	    $resource2->spend = 0;
    }
  	
  	$form[$first_day_month][$current_category->nid] = array (
      'spend' => array ( '#type' => 'textfield',
                         '#default_value' => $resource2->spend,
                         '#weight' => 1,
                         '#required' => TRUE,
                         '#prefix' => '<div class="outflow-personal-spend-field ' . $css_class . '">',
                         '#suffix' => '€</div>',       
                         ),
      'name' => array ( '#type' => 'hidden' , "#default_value" => $current_category->name ),                   
      'id' => array ( '#type' => 'hidden' , "#default_value" => $resource2->nid ),
    );
  }
  
  // Add submit button
  $form['submit'] = array (
      '#type' => 'submit',
      '#value' => t('Save'),
  );
  // Add theming function
  $form['#theme'] = 'finanzen_outflow_sonstiges_edit_month_form_theme';
  // This line is needed in order to keep the tree structure in form_state
  $form['#tree'] = TRUE;
  // Add redirection
  //$form['#redirect'] = 'finanzen/outflow/sonstiges/' . $first_day_month;
  $form['#redirect'] = 'finanzen/' . $first_day_month;
  
  return $form; 

}


function theme_finanzen_outflow_sonstiges_edit_month_form_theme ( $form ) {
     
  $header = array('name' => t("Name"), 'spend' => t("Spend"));
  
  //Get key value for the first array element
  $first_day_month = current(array_keys($form));
  
  $first_child = reset($form);
  
  foreach (element_children($first_child) as $category_id) {
    $rowdata = array();
    $rowdata[] = $form[$first_day_month][$category_id]['name']['#value'];
    $rowdata[] = drupal_render($form[$first_day_month][$category_id]['spend']);
    $row = array('data' => $rowdata);
    $rows[] = $row;
  }
  
  // This is where we create the table using theme()
  $output .= theme('table', $header, $rows, array('class' => 'finanzen'));
  
  $output .= drupal_render($form);
  return $output;
}


function finanzen_outflow_sonstiges_edit_month_form_validate ( $form, &$form_state ) {
  
  $first_day_month = current(array_keys($form));

  foreach (element_children($form_state['values'][$first_day_month]) as $category_id) {
  
    if (!(is_numeric($form_state['values'][$first_day_month][$category_id]['spend']))) {
      form_set_error("$first_day_month][$category_id][spend", t('Spend: You must enter a numeric value.'));
    }
  }
}


function finanzen_outflow_sonstiges_edit_month_form_submit ( $form, &$form_state )
{
  $first_day_month = current(array_keys($form));
  
  foreach (element_children($form_state[$first_day_month]) as $category_id) {
  	$spend = $form_state['values'][$first_day_month][$category_id]['spend'];
  	$reg_id = $form_state['values'][$first_day_month][$category_id]['id'];
  	
  	if ($reg_id) { // DB Register already exists
	    if ($spend == 0) {
		    $query_delete = "DELETE FROM finanzen_outflow_sonstiges_month WHERE nid='" . $reg_id . "'";
		    $queryResult = db_query ( $query_delete );	    
	    }
	    else { // $spend != 0
		   	$query = "UPDATE finanzen_outflow_sonstiges_month SET spend='" . $spend . "' 
		    		  WHERE nid='" . $reg_id . "'";
	        $queryResult = db_query ( $query );
	    }
    } 
    else { // DB Register does not exists
      if ($form_state['values'][$first_day_month][$category_id]['spend']!= 0) {
	    $query = "INSERT into {finanzen_outflow_sonstiges_month} ( category_id, date, spend )
                  values ( %d, '%s', %f )";
        $queryResult = db_query ( $query, $category_id, $first_day_month, $spend);   
      }
    }
  }
  
  if ($queryResult)
    drupal_set_message("Changes saved correctly", "status");
}

