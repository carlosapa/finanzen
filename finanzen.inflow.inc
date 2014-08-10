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
  
  $header = array('invoice_nr' => t("Invoice Nr"), 'project' => t("Project"), 
  				  'spend' => t("Spend"), 'taxrate' => t("Tax Rate"));
  
  //$query = "SELECT uid, name FROM users WHERE status=1 AND uid!=1";
  $query = "SELECT * FROM finanzen_inflow WHERE date=" . $first_day_month;
  $queryResult = db_query ($query);
    
  while ($current_invoice = db_fetch_object($queryResult)) {
  	$row = array( 'invoice_nr' => $current_invoice->invoice_nr,
  	  		      'project' => _finanzen_get_project_code($current_invoice->project_id),
  				  'spend' => $current_invoice->spend . ' €',
   				  'taxrate' => $current_invoice->taxrate . '%' );
   				  
  	$rows[] = $row;
  }
  
  $content .= theme_table($header, $rows , array('class' => 'finanzen'));
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
  $content .= '  <a href="/finanzen/inflow/edit-month/' . $date . '"><img class="edit-pencil" src="/' 
              . drupal_get_path('module', 'finanzen') . '/img/pencil.png" />edit</a>';
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
