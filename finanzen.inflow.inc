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
  else { $first_day_month = $date; }
  
  $content .= _finanzen_inflow_navigator($first_day_month);
  
  $header = array('invoice_nr' => t("Invoice Nr"), 
  				  'project' => t("Project"), 
  				  'spend' => t("Spend"), 
  				  'taxrate' => t("Tax Rate"),
  				  'edit' => t("Edit"), 
  				  'delete' => t("Delete"));
  
  $query = "SELECT * FROM finanzen_inflow WHERE date=" . $first_day_month;
  $queryResult = db_query ($query);

  $total_amount = 0;
  while ($current_invoice = db_fetch_object($queryResult)) {
    $row = array( 'invoice_nr' => $current_invoice->invoice_nr,
	   		      'project' => _finanzen_get_project_code($current_invoice->project_id),
	 			  'spend' => $current_invoice->spend . ' €',
 				  'taxrate' => $current_invoice->taxrate . '%',
 				  'edit' => '<a href="/finanzen/inflow/edit/' . 
   				  			   $current_invoice->nid . '/' . $first_day_month . '">edit</a>',
   				  'delete' => '<a href="/finanzen/inflow/delete/confirmation/' . 
   				  			   $current_invoice->nid . '/' . $first_day_month . '">delete</a>' );
	   				  
	$rows[] = $row;
	$total_amount += $current_invoice->spend;
  }
  
  $content .= theme_table($header, $rows , array('class' => 'finanzen inflow'));
  if (!$row)
   	$content .= "<p syle='margin-bottom: 20px;'>Still no invoices this month</p>";
  
  $content .= '<p><b>Total inflow ' . date('F Y', $first_day_month) . '</b>: ' . 
              number_format($total_amount, 2, ".", ".") . '€</p>';
  
  $content .= finanzen_inflow_add($first_day_month);
  
  return $content;
}

/**
 * This function outputs a Inflow month navigator
 */
function _finanzen_inflow_navigator($date)
{ 
  $current_date = time ();
  $current_first_day_month = _finanzen_first_day_month($current_date);
  // Init tool date -> 01.01.2014
  $initial_time = "1388530800";
  
  $content .= '<div class="navigator">';
  
  $content .= '<div class="navigator-date">';
  $content .= date('F Y', $date);
  $content .= "</div>";

  $content .= '<div class="navigator-next">';
  if ($date < $current_first_day_month) {
	$content .= '<a title="Go to next month" href="/finanzen/inflow/' 
	            . _finanzen_get_first_day_next_month($date) . '"> &#8811; </a>';
  }
  else { $content .= '<span class="muted"> &#8811; </span>'; }
  $content .= "</div>";
   
  $content .= '<div class="navigator-prev">';
  if ($date > $initial_time) {
  	$content .= '<a title="Go to previous month" href="/finanzen/inflow/' 
  	            . _finanzen_get_first_day_previous_month($date) . '"> &#8810; </a>';
  }
  else { $content .= '<span class="muted"> &#8810; </span>'; }
  $content .= "</div>";
  
  $content .= "</div>"; // class="navigator"
   
  return $content;     
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
  
  $query = "SELECT og.nid FROM og INNER JOIN node
			ON og.nid = node.nid";
//			WHERE node.status=1";
  $queryResult = db_query ($query);
  
  while ($project_id = db_fetch_object($queryResult))
  	$projects[$project_id->nid] = _finanzen_get_project_code($project_id->nid);

  $projects['0'] = 'NONE';
 
  $form = array (
    'inflow_add' => array (
      '#type' => 'fieldset',
      '#title' => t( 'Add new invoice for <b>' . $human_date . '</b>' ),
      'invoice_nr' => array ( '#type' => 'textfield',
                              '#title' => 'Invoice Nr',
                              '#required' => TRUE,
                              '#prefix' => '<div class="inflow_add to-left bleed-right">',
                              '#suffix' => '</div>',
                            ),        
      'project' => array ( '#type' => 'select',
                           '#title' => 'Project',
                           '#options' => $projects,
                           '#required' => TRUE,
                           '#prefix' => '<div class="inflow_add to-left bleed-right">',
                           '#suffix' => '</div>',
                         ),
      'spend' => array ( '#type' => 'textfield',
                         '#title' => 'Spend in €',
                         '#required' => TRUE,
                         '#prefix' => '<div class="inflow_add to-left bleed-right">',
                         '#suffix' => '</div>',
                       ),
      'taxrate' => array ( '#type' => 'select',
                           '#title' => 'Tax rate',
                           '#options' => array('0' => '0%', '7' => '7%', '19' => '19%'),
                           '#required' => TRUE,
                           '#prefix' => '<div class="inflow_add to-left ">',
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
    form_set_error("spend", t('Spend: You must enter a numeric value.'));
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
  $human_date = date('F Y', $current_invoice->date);
  
  $query2 = "SELECT og.nid FROM og INNER JOIN node
			ON og.nid = node.nid"; 
//			WHERE node.status=1";
  $queryResult2 = db_query ($query2);
  
  while ($project_id = db_fetch_object($queryResult2))
  	$projects[$project_id->nid] = _finanzen_get_project_code($project_id->nid);

  $projects['0'] = 'NONE';
 
  $form = array (
    'inflow_edit' => array (
      '#type' => 'fieldset',
      '#title' => t( 'Edit invoice Nr <b>' . $current_invoice->invoice_nr . '</b>' ),
      'invoice_nr' => array ( '#type' => 'textfield',
                              '#title' => 'Invoice Nr',
                              '#default_value' => $current_invoice->invoice_nr,
                              '#required' => TRUE,
                              '#prefix' => '<div class="inflow_add to-left bleed-right">',
                              '#suffix' => '</div>',
                            ),        
      'project' => array ( '#type' => 'select',
                           '#title' => 'Project',
                           '#options' => $projects,
                           '#default_value' => $current_invoice->project_id,
                           '#required' => TRUE,
                           '#prefix' => '<div class="inflow_add to-left bleed-right">',
                           '#suffix' => '</div>',
                         ),
      'spend' => array ( '#type' => 'textfield',
                         '#title' => 'Spend in €',
                         '#default_value' => $current_invoice->spend,
                         '#required' => TRUE,
                         '#prefix' => '<div class="inflow_add to-left bleed-right">',
                         '#suffix' => '</div>',
                       ),
      'taxrate' => array ( '#type' => 'select',
                           '#title' => 'Tax rate',
                           '#options' => array('0' => '0%', '7' => '7%', '19' => '19%'),
                           '#default_value' => $current_invoice->taxrate,
                           '#required' => TRUE,
                           '#prefix' => '<div class="inflow_add to-left ">',
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
  
  $query = "UPDATE finanzen_inflow SET 
  			invoice_nr='" . $invoice_nr . "', 
  			project_id=" . $project_id . ",  
  			spend=" . $spend . ",  
  			taxrate=" . $taxrate . "
  			WHERE nid=" . $id;
  $queryResult = db_query ( $query );
  if ($queryResult)
    drupal_set_message("Invoice modified correctly", "status");
  
}


function finanzen_inflow_edit_form_validate($form, &$form_state) 
{
  if (!is_numeric($form_state['values']['spend'])) {
    form_set_error("spend", t('Spend: You must enter a numeric value.'));
  }
}

/* INFLOW delete with confirmation */

function finanzen_inflow_delete_confirmation ( $id, $date = 0 )
{
  $query = "SELECT invoice_nr FROM finanzen_inflow WHERE nid=" . $id;
  $queryResult = db_query ($query);
  $incoice_nr = db_fetch_object($queryResult);
  drupal_set_message ( 'Invoice number ' . $invoice_nr . ' is going to be deleted. This action cannot be undone. 
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
