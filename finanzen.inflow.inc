<?php 

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
  
  $header = array('invoice_nr' => t("Rechnung Nr."), 
  				  'project' => t("Projekt"), 
  				  'date'  => t("Datum"),
  				  'spend' => t("Betrag"), 
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
  //$current_date = time ();
  //$current_first_day_month = _finanzen_first_day_month($current_date);
  
  $content .= '<div class="navigator">';
  
  $content .= '<div class="navigator-back"><a href="/finanzen/' . $date . '"> &#8592; zurück</a></div>';
  
  $content .= '<div class="navigator-date">';
  $content .= date('F Y', $date);
  $content .= "</div>";

  $content .= '<div class="navigator-next">';
  //if ($date < $current_first_day_month) {
  $content .= '<a title="Go to next month" href="/finanzen/inflow/' 
	          . _finanzen_get_first_day_next_month($date) . '"> &#8811; </a>';
  //}
  //else { $content .= '<span class="muted"> &#8811; </span>'; }
  $content .= "</div>";
   
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


function finanzen_inflow_all ()
{  
  $header = array('invoice_nr' => t("Rechnung Nr."), 
  				  'project' => t("Projekt"),
  				  'date'  => t("Datum"),
  				  'spend' => t("Betrag"), 
  				  'taxrate' => t("MwSt"),
  				  'edit' => t("Edit"), 
  				  'delete' => t("Delete"),
  				  'status' =>t("Status"));
  
  $query = "SELECT * FROM finanzen_inflow ORDER BY date DESC";
  $queryResult = db_query ($query);

  $total_amount = 0;
  while ($current_invoice = db_fetch_object($queryResult)) {
  
    if ($current_invoice->paid == 0)
      $status_paid = "unbezahlt";
    else 
      $status_paid = "bezahlt";
      
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
  
  $content .= theme_table($header, $rows , array('class' => 'finanzen inflow all-invoices'));
  if (!$row)
   	$content .= "<p syle='margin-bottom: 20px;'>Still no invoices this month</p>";
  
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
  
  $query = "SELECT og.nid FROM og 
            INNER JOIN node
			ON og.nid = node.nid";
            //WHERE node.status=1";
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
  
  $query2 = "SELECT og.nid FROM og 
             INNER JOIN node
			 ON og.nid = node.nid"; 
             //WHERE node.status=1";
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
