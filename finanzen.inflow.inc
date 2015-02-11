<?php 

/**
 * Jahresübersicht Ausgangsrechnungen
 */
function finanzen_inflow_general ( $date = 0 )
{
  if ($date == 0) {
    $current_date = time ();
    $first_day_month = _finanzen_first_day_month($current_date);
  }
  else { $first_day_month = $date; }

  $content .= _finanzen_inflow_general_navigator($first_day_month);
  $content .= finanzen_reports_inflow($first_day_month);
  
  return $content;
}

/**
 * This function outputs a Inflow month navigator
 */
function _finanzen_inflow_general_navigator($date)
{ 
  $content .= '<div class="navigator">';
  $content .= '<div class="navigator-back"><a href="/finanzen/' . $date . '"> &#8592; zurück</a></div>';
  $content .= '<div class="navigator-title bold black">Jahresübersicht Ausgangsrechnungen</div>';
  $content .= '<div class="navigator-next">';
  
  $query = "SELECT date FROM finanzen_inflow ORDER BY date DESC LIMIT 1";
  $queryResult = db_query ($query);
  
  if (db_affected_rows()== 1) {
  	$invoice = db_fetch_object($queryResult);
  	$last_invoice_date = $invoice->date;
  }
  
  if ($last_invoice_date >= _finanzen_get_first_day_next_year($date)) {
    $content .= '<a title="Go to next year" href="/finanzen/inflow/general/' 
	            . _finanzen_get_first_day_next_year($date) . '"> &#8811; </a>';
  }
  else 
    $content .= '<span class="muted"> &#8811; </span>';
    
  $content .= "</div>";
  
  $content .= '<div class="navigator-date black">' . date('Y', $date) . "</div>";
   
  $content .= '<div class="navigator-prev">';
  if ($date > ABSOLUTE_INITIAL_TIME) {
  	$content .= '<a title="Go to previous year" href="/finanzen/inflow/general/' 
  	            . _finanzen_get_first_day_previous_year($date) . '"> &#8810; </a>';
  }
  else
    $content .= '<span class="muted"> &#8810; </span>';
  
  $content .= "</div>";
  
  $content .= "</div>"; // class="navigator"
   
  return $content;     
}


function finanzen_reports_inflow ( $date = 0 ) {
  
  if ($date == 0) {
    $current_date = time ();
    $first_day_month = _finanzen_first_day_month($current_date);
  }
  else { $first_day_month = $date; }
  
  $last_row = array();
  $paid_invoices_row = array();
  $unpaid_invoices_row = array();
  $total_all_projects_year = 0;
  $total_all_projects_year_unpaid = 0;
  $total_all_projects_year_paid = 0;
  
  $months_curr_year = _finanzen_list_months_year ($first_day_month, 3);
  $header = array('vertrag' => array('data' => t("Vertrag"),'class' => 'bold column_money')) + 
    	    //array('vergangenerg' => array('data' => t("Vergangene Rg."),'class' => 'column_money')) + 
  			array('zpgesamt' => array('data' => t("ZP Gesamt"),'class' => 'bold column_money')) + 
  			array('project' => array('data' => t("Projekt"),'class' => 'column_project')) + 
            $months_curr_year + array('total' => array('data' => t("Gesamt"),
                                                       'class' => 'bold column_money'));
  
  $desplazamiento_tabla['vertrag']= array('data' => "",'class' => 'white');
  //$desplazamiento_tabla['vergangenerg']= array('data' => "",'class' => 'white');
  $desplazamiento_tabla['zpgesamt']= array('data' => "",'class' => 'white');
                    
  $query = "SELECT DISTINCT project_id FROM finanzen_inflow
    		INNER JOIN content_type_group ON content_type_group.nid = finanzen_inflow.project_id
  			WHERE date>=" . _finanzen_first_day_year($first_day_month) . " 
  			AND date<" . _finanzen_get_first_day_next_year($first_day_month) . "
  			ORDER BY content_type_group.field_project_code_value";
  $queryResult = db_query ( $query );
  
  while ($project = db_fetch_object($queryResult)){
  
  	$row = array();
  	
  	$query3 = "SELECT * FROM finanzen_inflow_contract_project WHERE project_id=" . $project->project_id;
  	$queryResult3 = db_query($query3);
  	$gesamt_vertrag_nachtraege_proj = 0;
  	if (db_affected_rows() > 0) {
	    while ($outflow = db_fetch_object($queryResult3)) {
	      $gesamt_vertrag_nachtraege_proj += $outflow->spend;
	    }
    }  	
    $row['vertrag']= array('data' => _finanzen_format_number($gesamt_vertrag_nachtraege_proj), 'class' => 'bold not-empty');
    
    $query4 = "SELECT * FROM finanzen_inflow WHERE project_id=" . $project->project_id;
  	$queryResult4 = db_query($query4);
  	$gesamt_inflow_proj = 0;
  	//$past_inflow_proj = 0;
  	if (db_affected_rows() > 0) {
	    while ($outflow = db_fetch_object($queryResult4)) { 
	      $gesamt_inflow_proj += $outflow->spend;
	      //if ($outflow->date < _finanzen_first_day_year($date))
	      //  $past_inflow_proj += $outflow->spend;
	    }
    }
    
    //$row['vergangenerg']= array('data' => _finanzen_format_number($past_inflow_proj), 'class' => 'not-empty');
    
    if ($gesamt_vertrag_nachtraege_proj == $gesamt_inflow_proj)
      $row['zpgesamt']= array('data' => _finanzen_format_number($gesamt_inflow_proj), 'class' => 'bold not-empty');
    else
      $row['zpgesamt']= array('data' => _finanzen_format_number($gesamt_inflow_proj), 'class' => 'error bold not-empty'); 
    
    $row['project']= array('data' => _finanzen_link_to_project($project->project_id, $first_day_month),  
                                    'class' => 'not-empty');
    $last_row['name'] = "Gesamt";
    $unpaid_invoices_row['name'] = "Offene Rg.";
    $paid_invoices_row['name'] = "Eingezahlte Rg.";
    
    $total_project_year = 0;
    	
	foreach ($months_curr_year as $month_name => $month_data ) {
		
		$total_project_month = 0;
		$total_project_month_paid = 0;
		$total_project_month_unpaid = 0;
		
		if (empty($last_row[$month_name])): $last_row[$month_name] = 0; endif;
		if (empty($unpaid_invoices_row[$month_name])): $unpaid_invoices_row[$month_name] = 0; endif;
		if (empty($paid_invoices_row[$month_name])): $paid_invoices_row[$month_name] = 0; endif;
		
		$query2 = "SELECT * from finanzen_inflow
				   WHERE project_id=" . $project->project_id . " 
				   AND date>=" . $month_data['date'] . "
				   AND date<" . _finanzen_get_first_day_next_month($month_data['date']);
		$queryResult2 = db_query($query2);
		
		$all_month_invoices_paid = TRUE;
		while ($outflow = db_fetch_object($queryResult2)) {
			$total_project_month += $outflow->spend;
			$last_row[$month_name] += $outflow->spend;
			
			if ($outflow->paid) {
			  $paid_invoices_row[$month_name] += $outflow->spend;
			  $total_project_month_paid += $outflow->spend;
			}
			else {
			  $unpaid_invoices_row[$month_name] += $outflow->spend;
			  $total_project_month_unpaid += $outflow->spend;
			}
			
			$total_project_year += $outflow->spend;
			$all_month_invoices_paid = ($all_month_invoices_paid && $outflow->paid);
		}
		
		if ($total_project_month > 0) {
  		  if ($month_data['date'] > _finanzen_first_day_month(time()))
  		    $row[] = array('data' => _finanzen_format_number($total_project_month), 'class' => 'not-empty prognose');
  		  else {
	  		  if (($total_project_month_paid > 0) && ($total_project_month_unpaid > 0)) {
		  		   $row[] = array('data' => _finanzen_format_number($total_project_month_paid) . '</br>' . 
		  		                            '<span class=unbezahlt>' . _finanzen_format_number($total_project_month_unpaid) .'</span>', 								  'class' => 'not-empty');
	  		  }
	  		  elseif ($total_project_month_paid > 0)
	  		    $row[] = array('data' => _finanzen_format_number($total_project_month_paid), 'class' => 'not-empty');
	  		  else
	  		    $row[] = array('data' => _finanzen_format_number($total_project_month_unpaid), 'class' => 'not-empty unbezahlt'); 
	  		  
  		  }

  		  
  		  /*
  		  if ($all_month_invoices_paid)
  		    $row[] = array('data' => _finanzen_format_number($total_project_month), 'class' => 'not-empty');
  		  elseif ($month_data['date'] <= _finanzen_first_day_month(time()))
  		    $row[] = array('data' => _finanzen_format_number($total_project_month), 'class' => 'not-empty unbezahlt');
  		  else 
  		    $row[] = array('data' => _finanzen_format_number($total_project_month), 'class' => 'not-empty prognose');
  		  */
  		}
  		else
  		  $row[] = _finanzen_format_number(0);
	}
	
	$row['total'] = array('data' => _finanzen_format_number($total_project_year), 'class' => 'not-empty bold');
	$rows[] = $row;
	$total_all_projects_year += $total_project_year;

  }
  
  if ($unpaid_invoices_row) {
	  foreach ($unpaid_invoices_row as $month_name => $month_data){
	     if ($month_name != "name") {
			$unpaid_invoices_row[$month_name] = _finanzen_format_number($month_data);
			$total_all_projects_year_unpaid += $month_data;
		 }
	  }
	  $unpaid_invoices_row['total'] = _finanzen_format_number($total_all_projects_year_unpaid);
  }
      
  if ($paid_invoices_row) {  
	  foreach ($paid_invoices_row as $month_name => $month_data){
	     if ($month_name != "name") {
			$paid_invoices_row[$month_name] =_finanzen_format_number($month_data);
			$total_all_projects_year_paid += $month_data;
		 }
	  }
	  $paid_invoices_row['total'] =_finanzen_format_number($total_all_projects_year_paid);
  }
  
  if ($last_row) {
	  foreach ($last_row as $month_name => $month_data){
	     if ($month_name != "name")
			$last_row[$month_name] = _finanzen_format_number($month_data);
	  }
	  $last_row['total'] = _finanzen_format_number($total_all_projects_year);
  }
  
  $unpaid_invoices_row = $desplazamiento_tabla + $unpaid_invoices_row;
  $paid_invoices_row = $desplazamiento_tabla + $paid_invoices_row;
  $last_row = $desplazamiento_tabla + $last_row;
  
  $rows[] = array('data' => $unpaid_invoices_row, 'class' => 'bold light-grey not-empty');
  $rows[] = array('data' => $paid_invoices_row, 'class' => 'bold light-grey not-empty'); 
  $rows[] = array('data' => $last_row, 'class' => 'bold grey not-empty');
  
  $content .= theme_table($header, $rows, array('class' => 'finanzen reports inflow-general'));

  return $content;
}


/**
 *  Current month outflow personal
 */

function finanzen_inflow ( $date = 0 )
{
  if ($date == 0) {
    $current_date = time ();
    $first_day_month = _finanzen_first_day_month($current_date);
  }
  else { 
    $first_day_month = _finanzen_first_day_month($date); 
  }
  
  $first_day_next_month = _finanzen_get_first_day_next_month($first_day_month);
  
  $content .= _finanzen_inflow_navigator($first_day_month);
  
  $header = array('invoice_nr' => array('data' => t("Rechnung Nr."), 'class' => 'column_invoice_nr'), 
  				  'project' => t("Projekt"), 
  				  'date'  => t("Datum"),
  				  'spend' => array('data' => t("Betrag"), 'class' => 'column_money'),
  				  'taxrate' => t("MwSt"),
  				  'edit' => t("Edit"), 
  				  'delete' => t("Delete"),
  				  'status' => t("Status"));
  
  $query = "SELECT * FROM finanzen_inflow 
            WHERE date>=" . $first_day_month .
            " AND date<" . $first_day_next_month .
            " ORDER BY date ASC";
  $queryResult = db_query ($query);

  $total_amount = 0;
  while ($current_invoice = db_fetch_object($queryResult)) {
    if ($current_invoice->paid == 0)
      $status_paid = "unbezahlt";
    else 
      $status_paid = "bezahlt";
      
    $status = $status_paid . ' <a href="/finanzen/inflow/change_status/' . $current_invoice->nid . 
                             '?' . drupal_get_destination() . '">change</a>';
      
    //$status = $status_paid . ' <a href="/finanzen/inflow/change_status/' . $current_invoice->nid . '">change</a>';   
      
    $rowdata = array( 'invoice_nr' => $current_invoice->invoice_nr,
	   		          'project' => _finanzen_link_to_project($current_invoice->project_id, $first_day_month),
	   		          'date' => date('d.m.Y', $current_invoice->date),
	 			      'spend' => _finanzen_format_number($current_invoice->spend),
 				      'taxrate' => $current_invoice->taxrate . '%',
 				      'edit' => '<a href="/finanzen/inflow/edit/' . 
   				  			    $current_invoice->nid . '/' . $first_day_month . '">edit</a>',
   				      'delete' => '<a href="/finanzen/inflow/delete/confirmation/' . 
   				  			      $current_invoice->nid . '/' . $first_day_month . '">delete</a>',
   				      'status' => $status );
   				  
   	$row = array('data' => $rowdata, 'class' => $status_paid);
	   				  
	$rows[] = $row;
	$total_amount += $current_invoice->spend;
  }
  
  $content .= theme_table($header, $rows , array('class' => 'finanzen inflow all-invoices'));
  if (!$row)
   	$content .= "<p syle='margin-bottom: 20px;'>Still no invoices this month</p>";
  
  $content .= '<p><b>Total inflow ' . date('F Y', $first_day_month) . '</b>: ' . 
              _finanzen_format_number($total_amount) . '</p>';
  
  $content .= finanzen_inflow_add($first_day_month);
  
  return $content;
}

/**
 * This function outputs a Inflow month navigator
 */
function _finanzen_inflow_navigator($date)
{ 
  $content .= '<div class="navigator">';
  
  $content .= '<div class="navigator-back"><a href="/finanzen/inflow/general/' . $date . '"> &#8592; zurück</a></div>';

  $content .= '<div class="navigator-next">';
  //if ($date < $current_first_day_month) {
  $content .= '<a title="Go to next month" href="/finanzen/inflow/' 
	          . _finanzen_get_first_day_next_month($date) . '"> &#8811; </a>';
  //}
  //else { $content .= '<span class="muted"> &#8811; </span>'; }
  $content .= "</div>";
  
  $content .= '<div class="navigator-date black">' . date('F Y', $date) . '</div>';
   
  $content .= '<div class="navigator-prev">';
  if ($date > ABSOLUTE_INITIAL_TIME) {
  	$content .= '<a title="Go to previous month" href="/finanzen/inflow/' 
  	            . _finanzen_get_first_day_previous_month($date) . '"> &#8810; </a>';
  }
  else { $content .= '<span class="muted"> &#8810; </span>'; }
  $content .= "</div>";
  
  $content .= "</div>"; // class="navigator"
   
  return $content;     
}


function finanzen_inflow_all ( $date = 0 )
{ 
  if ($date == 0) {
    $current_date = time ();
    $first_day_year = _finanzen_first_day_year($current_date);
  }
  else { 
    $first_day_year = _finanzen_first_day_year($date); 
  }
  
  $header = array('invoice_nr' => array('data' => t("Rechnung Nr."), 'class' => 'column_invoice_nr'), 
  				  'project' => t("Projekt"),
  				  'date'  => t("Datum"),
  				  'spend' => array('data' => t("Betrag"), 'class' => 'column_money'),
  				  'taxrate' => t("MwSt"),
  				  'edit' => t("Edit"), 
  				  'delete' => t("Delete"),
  				  'status' =>t("Status"));
  
  $query = "SELECT * FROM finanzen_inflow 
            WHERE date>=" . $first_day_year . ' AND date<' . _finanzen_get_first_day_next_year($first_day_year) . " 
            ORDER BY date DESC";
  $queryResult = db_query ($query);

  $total_amount = 0;
  $total_paid = 0;
  $total_unpaid = 0;
  
  while ($current_invoice = db_fetch_object($queryResult)) {
  
    if ($current_invoice->paid == 0) {
      $status_paid = "unbezahlt";
      $total_unpaid += $current_invoice->spend;
    }
    else { 
      $status_paid = "bezahlt";
      $total_paid += $current_invoice->spend;
    }
    
    $total_amount += $current_invoice->spend;
    
    
    $status = $status_paid . ' <a href="/finanzen/inflow/change_status/' . $current_invoice->nid . 
                             '?' . drupal_get_destination() . '">change</a>';

    $rowdata = array( 'invoice_nr' => $current_invoice->invoice_nr,
	   		          'project' => _finanzen_link_to_project($current_invoice->project_id),
	   		          'date' => date('d.m.Y', $current_invoice->date),
	 			      'spend' => _finanzen_format_number($current_invoice->spend),
 				      'taxrate' => $current_invoice->taxrate . '%',
 				      'edit' => '<a href="/finanzen/inflow/edit/' . 
   				  		 	    $current_invoice->nid . '">edit</a>',
   				      'delete' => '<a href="/finanzen/inflow/delete/confirmation/' . 
   				  			      $current_invoice->nid . '">delete</a>',
   				      'status' => $status );

    $row = array('data' => $rowdata, 'class' => $status_paid);
	   				  
	$rows[] = $row;
  }
  
  /* --- RESUMEN DEL AÑO --- */
  $rowdata2 = array('concept' => "Gestellte Rechnungen",'betrag' => _finanzen_format_number($total_amount));
  $rows2[] = array('data' => $rowdata2, 'class' => "not-empty light-grey");
  $rowdata2 = array('concept' => "Bezahlte Rechnungen",'betrag' => _finanzen_format_number($total_paid));
  $rows2[] = array('data' => $rowdata2, 'class' => "not-empty light-grey");
  $rowdata2 = array('concept' => "Unbezahlte Rechnungen",'betrag' => _finanzen_format_number($total_unpaid));
  $rows2[] = array('data' => $rowdata2, 'class' => "not-empty light-grey");
  
  $content .= theme_table(array(), $rows2, array('class' => 'finanzen-klein reports'));
  
  $content .= _finanzen_inflow_all_navigator($first_day_year);
  
  $content .= theme_table($header, $rows , array('class' => 'finanzen inflow all-invoices'));
  
  if (!$row)
   	$content .= "<p syle='margin-bottom: 20px;'>Still no invoices this month</p>";
  
  return $content;
}

function _finanzen_inflow_all_navigator($date)
{ 
  $content .= '<div class="navigator">';
  $content .= '<div class="navigator-back"><a href="/finanzen/' . $date . '"> &#8592; zurück</a></div>';
  $content .= '<div class="navigator-title bold black">Zusammenstellung der Rechnungen</div>';
  $content .= '<div class="navigator-next">';
  
  $query = "SELECT date FROM finanzen_inflow ORDER BY date DESC LIMIT 1";
  $queryResult = db_query ($query);
  
  if (db_affected_rows()== 1) {
  	$invoice = db_fetch_object($queryResult);
  	$last_invoice_date = $invoice->date;
  }
  
  if ($last_invoice_date >= _finanzen_get_first_day_next_year($date)) {
    $content .= '<a title="Go to next year" href="/finanzen/inflow/all/' 
	            . _finanzen_get_first_day_next_year($date) . '"> &#8811; </a>';
  }
  else 
    $content .= '<span class="muted"> &#8811; </span>';
    
  $content .= "</div>";

  $content .= '<div class="navigator-date black">' . _finanzen_get_year($date) . '</div>';
   
  $content .= '<div class="navigator-prev">';
  if ($date > ABSOLUTE_INITIAL_TIME) {
  	$content .= '<a title="Go to previous month" href="/finanzen/inflow/all/' 
  	            . _finanzen_get_first_day_previous_month($date) . '"> &#8810; </a>';
  }
  else { $content .= '<span class="muted"> &#8810; </span>'; }
  $content .= "</div>";

  $content .= "</div>";
     
  return $content;     
}


function finanzen_inflow_change_payment_status($invoice_id) {
  
  $query = "SELECT paid, invoice_nr FROM finanzen_inflow WHERE nid=" . $invoice_id;
  $queryResult = db_query ( $query );
  $invoice = db_fetch_object($queryResult);
  
  if ($invoice->paid == 0) {
    $new_status = 1;
    $name_new_status = "bezahlt";
  }
  else {
    $new_status = 0;
    $name_new_status = "unbezahlt";
  }
  
  $query2 = "UPDATE finanzen_inflow SET paid=" . $new_status . " WHERE nid=" . $invoice_id;
  $queryResult2 = db_query ( $query2 );
  if ($queryResult2)
    drupal_set_message("Invoice " . $invoice->invoice_nr . " mark as " . $name_new_status , "status");

  drupal_goto();
}


/* ----------------------------------------------------------------------------- */
/* ----------------------------- INSERT INVOICE  ------------------------------- */
/* ----------------------------------------------------------------------------- */

function finanzen_inflow_add ( $date )
{
  $content .= drupal_get_form ( 'finanzen_inflow_add_form', $date );
  return $content;
}


function finanzen_inflow_add_form ( $form, $date )
{
  $human_date = date('F Y', $date);
  
  //$query = "SELECT og.nid FROM og 
  //          INNER JOIN node
  //	  	  ON og.nid = node.nid";
  //          WHERE node.status=1";
            
  $query = "SELECT og.nid FROM og 
    		INNER JOIN content_type_group ON content_type_group.nid = og.nid
  			ORDER BY content_type_group.field_project_code_value";
  $queryResult = db_query ($query);
  
  while ($project_id = db_fetch_object($queryResult))
  	$projects[$project_id->nid] = _finanzen_get_project_code($project_id->nid);

  $projects['0'] = 'NONE';
 
  $form = array (
    'inflow_add' => array (
      '#type' => 'fieldset',
      '#title' => t( 'Add new invoice for <b>' . $human_date . '</b>' ),
      'invoice_nr' => array ( '#type' => 'textfield',
                              '#title' => 'Rechnung Nr.',
                              '#required' => TRUE,
                              '#prefix' => '<div class="inflow-add to-left bleed-right">',
                              '#suffix' => '</div>',
                            ),        
      'project' => array ( '#type' => 'select',
                           '#title' => 'Projekt',
                           '#options' => $projects,
                           '#required' => TRUE,
                           '#prefix' => '<div class="inflow-add to-left bleed-right">',
                           '#suffix' => '</div>',
                         ),
      'spend' => array ( '#type' => 'textfield',
                         '#title' => 'Betrag €',
                         '#required' => TRUE,
                         '#prefix' => '<div class="inflow-add to-left bleed-right">',
                         '#suffix' => '</div>',
                       ),
      'taxrate' => array ( '#type' => 'select',
                           '#title' => 'MwSt',
                           '#options' => array('0' => '0%', '7' => '7%', '19' => '19%'),
                           '#required' => TRUE,
                           '#prefix' => '<div class="inflow-add to-left ">',
                           '#suffix' => '</div>',
                         ),
      'date' => array ( '#type' => 'hidden', "#default_value" => $date ),
      'submit' => array ( '#type' => 'submit',  
      					  '#value' => t('Save'), 
      					  '#prefix' => '<div class="clear-left bleed-left">',
                          '#suffix' => '</div>',),
    ),
  );
  
  $form['#redirect'] = 'finanzen/inflow/' . _finanzen_first_day_month($date);
  
  return $form;
}

function finanzen_inflow_add_form_submit ( $form, &$form_state )
{
  $invoice_nr = $form_state['values']['invoice_nr'];
  $project_id = $form_state['values']['project'];
  $spend = $form_state['values']['spend'];
  $taxrate = $form_state['values']['taxrate'];
  $date = $form_state['values']['date'];
  
  $query = "INSERT into finanzen_inflow ( invoice_nr, project_id, spend, taxrate, date )
            values ( '%s', %d, %f, %d, '%s' )";
  $queryResult = db_query ( $query, $invoice_nr, $project_id, $spend, $taxrate, $date );
}


function finanzen_inflow_add_form_validate($form, &$form_state) 
{
  if (!is_numeric($form_state['values']['spend'])) {
    form_set_error("spend", t('Betrag: You must enter a numeric value.'));
  }
}

/* ----------------------------------------------------------------------------- */
/* ------------------------------- EDIT INVOICE  ------------------------------- */
/* ----------------------------------------------------------------------------- */

function finanzen_inflow_edit ( $id )
{
  $content .= drupal_get_form ( 'finanzen_inflow_edit_form', $id );
  return $content;
}


function finanzen_inflow_edit_form ( $form, $id )
{
  $query = "SELECT * FROM finanzen_inflow WHERE nid=" . $id;
  $queryResult = db_query ($query);
  $current_invoice = db_fetch_object($queryResult);
  $dateformat = 'd.m.Y';
  //$human_date = date($dateformat, $current_invoice->date);
  
  //$query2 = "SELECT og.nid FROM og 
  //           INNER JOIN node
  //		   ON og.nid = node.nid"; 
  //           WHERE node.status=1";
  
  $query2 = "SELECT og.nid FROM og 
    		 INNER JOIN content_type_group ON content_type_group.nid = og.nid
  			 ORDER BY content_type_group.field_project_code_value";           
  $queryResult2 = db_query ($query2);
  
  while ($project_id = db_fetch_object($queryResult2))
  	$projects[$project_id->nid] = _finanzen_get_project_code($project_id->nid);

  $projects['0'] = 'NONE';
  

 
  $form = array (
    'inflow_edit' => array (
      '#type' => 'fieldset',
      '#title' => t( 'Edit Rechnung Nr. <b>' . $current_invoice->invoice_nr . '</b>' ),
      'invoice_nr' => array ( '#type' => 'textfield',
                              '#title' => 'Rechnung Nr.',
                              '#default_value' => $current_invoice->invoice_nr,
                              '#required' => TRUE,
                              '#prefix' => '<div class="inflow-add to-left bleed-right">',
                              '#suffix' => '</div>',
                            ),        
      'project' => array ( '#type' => 'select',
                           '#title' => 'Projekt',
                           '#options' => $projects,
                           '#default_value' => $current_invoice->project_id,
                           '#required' => TRUE,
                           '#prefix' => '<div class="inflow-add to-left bleed-right">',
                           '#suffix' => '</div>',
                         ),
      'spend' => array ( '#type' => 'textfield',
                         '#title' => 'Betrag €',
                         '#default_value' => $current_invoice->spend,
                         '#required' => TRUE,
                         '#prefix' => '<div class="inflow-add to-left bleed-right">',
                         '#suffix' => '</div>',
                       ),
      'taxrate' => array ( '#type' => 'select',
                           '#title' => 'MwSt',
                           '#options' => array('0' => '0%', '7' => '7%', '19' => '19%'),
                           '#default_value' => $current_invoice->taxrate,
                           '#required' => TRUE,
                           '#prefix' => '<div class="inflow-add to-left ">',
                           '#suffix' => '</div>',
                         ),
      'date'  => array ( '#type' => 'date_popup',
                         '#title' => 'Datum',
                         '#date_format' => $dateformat,
                         '#default_value' => date("Y-m-d", $current_invoice->date),
                         '#required' => TRUE,
                         '#date_year_range' => '-1:+1',
                         '#prefix' => '<div class="inflow-add to-left ">',
                         '#suffix' => '</div>',
                       ),

      'id' => array ( '#type' => 'hidden', "#default_value" => $current_invoice->nid ),
      'submit' => array ( '#type' => 'submit',  
      					  '#value' => t('Save'), 
      					  '#prefix' => '<div class="clear-left bleed-left">',
                          '#suffix' => '</div>',),
    ),
  );
  
  $form['#redirect'] = 'finanzen/inflow/' . $current_invoice->date;
  
  return $form;
}

function finanzen_inflow_edit_form_submit ( $form, &$form_state )
{
  $invoice_nr = $form_state['values']['invoice_nr'];
  $project_id = $form_state['values']['project'];
  $spend = $form_state['values']['spend'];
  $taxrate = $form_state['values']['taxrate'];
  $id = $form_state['values']['id'];
  $date = strtotime($form_state['values']['date']);
  
  $query = "UPDATE finanzen_inflow SET 
  			invoice_nr='" . $invoice_nr . "', 
  			project_id=" . $project_id . ", 
  			spend=" . $spend . ",  
  			taxrate=" . $taxrate . ",
  			date=" . $date . "
  			WHERE nid=" . $id;
  
  $queryResult = db_query ( $query );
  
  if ($queryResult)
    drupal_set_message("Invoice modified correctly", "status");
 
}


function finanzen_inflow_edit_form_validate($form, &$form_state) 
{
  if (!is_numeric($form_state['values']['spend'])) {
    form_set_error("spend", t('Betrag: You must enter a numeric value.'));
  }
}

/* INFLOW delete with confirmation */

function finanzen_inflow_delete_confirmation ( $id, $date = 0 )
{
  $query = "SELECT invoice_nr FROM finanzen_inflow WHERE nid=" . $id;
  $queryResult = db_query ($query);
  $invoice = db_fetch_object($queryResult);
  drupal_set_message ( 'Invoice number ' . $invoice->invoice_nr . ' is going to be deleted. This action cannot be undone. 
                        <a href="/finanzen/inflow/delete/' .  $id . '/' . $date . '">Confirm</a>', "warning");
  return finanzen_inflow($date);
}

function finanzen_inflow_delete ( $id, $date = 0 )
{
  $query = "DELETE from finanzen_inflow WHERE nid=" . $id ;
  $queryResult = db_query ( $query );
  drupal_set_message ( "Invoice deleted.", "status");
  return finanzen_inflow($date);
}
