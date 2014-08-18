<?php 

/**
 *  Current month outflow personal
 */

function finanzen_outflow_personal ( $date = 0 )
{
  if ($date == 0) {
    $current_date = time ();
    $first_day_month = _finanzen_first_day_month($current_date);
  }
  else { $first_day_month = $date; }
  
  $content .= _finanzen_outflow_personal_navigator($first_day_month);
  
  $header = array('name' => t("Name"), 'spend' => t("Spend"), 'taxrate' => t("Tax Rate") );
  
  //$query = "SELECT uid, name FROM users WHERE status=1 AND uid!=1";
  $query = "SELECT users.uid, users.name FROM users 
  			LEFT JOIN users_roles ON users.uid=users_roles.uid
  			WHERE users.uid!=1 AND users.uid!=0
  			ORDER BY users_roles.rid DESC, users.name";
  
  $queryResult = db_query ($query);
  
  $total_amount = 0;  
  
  while ($current_user = db_fetch_object($queryResult)) {
    
    $query2 = "SELECT nid, spend, taxrate FROM finanzen_outflow_worker_month 
    		   WHERE uid=" . $current_user->uid . " AND date=" . $first_day_month;
    $queryResult2 = db_query ($query2);
    $resource2 = db_fetch_object ($queryResult2);
    
    if (!$resource2) {
      $resource2->spend = 0;
      $resource2->taxrate = 0;
    }
    elseif ($resource2->spend == 0) {
    	// Si el gasto es 0 (tras una edicion) borramos en registro de la BD
    	$query_delete = "DELETE FROM finanzen_outflow_worker_month WHERE nid='" . $resource2->nid . "'";
    	$query_deleteResult = db_query ( $query_delete );	 
    	$resource2->taxrate = 0;   
    }
        
  	$total_amount += $resource2->spend;
  	
  	$row = array( 'name' => $current_user->name,
  				  'spend' => $resource2->spend . ' €',
   				  'taxrate' => $resource2->taxrate . '%' );
  	$rows[] = $row;
  	
  }
  
  $content .= theme_table($header, $rows , array('class' => 'finanzen outflow-personal'));
  
  $content .= '<p><b>Total outflow personal ' . date('F Y', $first_day_month) . '</b>: ' . 
              number_format($total_amount, 2, ".", ".") . '€</p>';

  return $content;
}

/**
 * This function outputs a Finanzen Outflow month navigator
 */
function _finanzen_outflow_personal_navigator($date)
{ 
  $current_date = time ();
  $current_first_day_month = _finanzen_first_day_month($current_date);
  // Init tool date -> 01.01.2014
  $initial_time = "1388530800";
  
  $content .= '<div class="navigator">';
  
  $content .= '<div class="navigator-date">';
  $content .= date('F Y', $date);
  $content .= '  <a href="/finanzen/outflow/personal/edit-month/' . $date . '"><img class="edit-pencil" src="/' 
              . drupal_get_path('module', 'finanzen') . '/img/pencil.png" />edit</a>';
  $content .= "</div>";

  
  $content .= '<div class="navigator-next">';
  if ($date < $current_first_day_month) {
	$content .= '<a title="Go to next month" href="/finanzen/outflow/personal/' 
	            . _finanzen_get_first_day_next_month($date) . '"> &#8811; </a>';
  }
  else { $content .= '<span class="muted"> &#8811; </span>'; }
  $content .= "</div>";
   
  $content .= '<div class="navigator-prev">';
  if ($date > $initial_time) {
  	$content .= '<a title="Go to previous month" href="/finanzen/outflow/personal/' 
  	            . _finanzen_get_first_day_previous_month($date) . '"> &#8810; </a>';
  }
  else { $content .= '<span class="muted"> &#8810; </span>'; }
  $content .= "</div>";
  
  $content .= "</div>"; // class="navigator"
   
  return $content;     
}

/* --------------------- Finanzen Personal Outflow Edit month --------------------------------- */

/**
 * Navigator
 */
function _finanzen_outflow_personal_edit_month_navigator ( $date ) 
{
  $content .= '<div class="navigator">';
  //$content .= '<div class="navigator-back"><a href="/finanzen/outflow/personal/' . $date . '"> &#8592; Back</a></div>';
  $content .= '<div class="navigator-back"><a href="/finanzen/' . $date . '"> &#8592; Back</a></div>';
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
  //$form['#redirect'] = 'finanzen/outflow/personal/' . $first_day_month;
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
  	
  	if ($reg_id != 0) { // DB Register already exists
	    $query = "UPDATE finanzen_outflow_worker_month SET spend=" . $spend . ", taxrate=" . $taxrate . 
                 " WHERE nid='" . $reg_id . "'";
        $queryResult = db_query ( $query );
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
/* ---------------------------- OUTFLOW SONSTIGES ------------------------------ */
/* ----------------------------------------------------------------------------- */


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

/**
 *  Current month outflow sonstiges
 */

function finanzen_outflow_sonstiges ( $date = 0 )
{
  if ($date == 0) {
    $current_date = time ();
    $first_day_month = _finanzen_first_day_month($current_date);
  }
  else { $first_day_month = $date; }
  
  $content .= _finanzen_outflow_sonstiges_navigator($first_day_month);
  
  $header = array('name' => t("Name"), 'spend' => t("Spend"));
  
  $query = "SELECT * FROM finanzen_outflow_sonstiges_categories WHERE active=1";
  $queryResult = db_query ($query);
  
  $total_amount = 0;
  while ($current_category = db_fetch_object($queryResult)) {
    $query2 = "SELECT nid, spend FROM finanzen_outflow_sonstiges_month 
    		   WHERE category_id=" . $current_category->nid . " AND date=" . $first_day_month;
    $queryResult2 = db_query ($query2);
    $resource2 = db_fetch_object ($queryResult2);
    
    if (!$resource2) {
      $resource2->spend = 0;
    }
    elseif ($resource2->spend == 0) {
    	// Si el gasto es 0 (tras una edicion) borramos en registro de la BD
    	$query_delete = "DELETE FROM finanzen_outflow_sonstiges_month WHERE nid='" . $resource2->nid . "'";
    	$query_deleteResult = db_query ( $query_delete );	    
    }
  	
  	$total_amount += $resource2->spend;
  	
  	$row = array( 'name' => $current_category->name,
  				  'spend' => $resource2->spend . ' €');
  	$rows[] = $row;
  }
  
  $content .= theme_table($header, $rows , array('class' => 'finanzen outflow-sonstiges'));
  
  $content .= '<p><b>Total outflow personal ' . date('F Y', $first_day_month) . '</b>: ' . 
              number_format($total_amount, 2, ".", ".") . '€</p>';
              
  return $content;
}

/**
 * This function outputs a Finanzen Outflow month navigator
 */
function _finanzen_outflow_sonstiges_navigator($date)
{ 
  $current_date = time ();
  $current_first_day_month = _finanzen_first_day_month($current_date);
  // Init tool date -> 01.01.2014
  $initial_time = "1388530800";
  
  $content .= '<div class="navigator">';
  
  $content .= '<div class="navigator-date">';
  $content .= date('F Y', $date);
  $content .= '  <a href="/finanzen/outflow/sonstiges/edit-month/' . $date . '"><img class="edit-pencil" src="/' 
              . drupal_get_path('module', 'finanzen') . '/img/pencil.png" />edit</a>';
  $content .= "</div>";

  
  $content .= '<div class="navigator-next">';
  if ($date < $current_first_day_month) {
	$content .= '<a title="Go to next month" href="/finanzen/outflow/sonstiges/' 
	            . _finanzen_get_first_day_next_month($date) . '"> &#8811; </a>';
  }
  else { $content .= '<span class="muted"> &#8811; </span>'; }
  $content .= "</div>";
   
  $content .= '<div class="navigator-prev">';
  if ($date > $initial_time) {
  	$content .= '<a title="Go to previous month" href="/finanzen/outflow/sonstiges/' 
  	            . _finanzen_get_first_day_previous_month($date) . '"> &#8810; </a>';
  }
  else { $content .= '<span class="muted"> &#8810; </span>'; }
  $content .= "</div>";
  
  $content .= "</div>"; // class="navigator"
   
  return $content;     
}

/* --------------------- Finanzen sonstiges Outflow Edit month --------------------------------- */

/**
 * Navigator
 */
function _finanzen_outflow_sonstiges_edit_month_navigator ( $date ) 
{
  $content .= '<div class="navigator">';
  //$content .= '<div class="navigator-back"><a href="/finanzen/outflow/sonstiges/' . $date . '"> &#8592; Back</a></div>';
  $content .= '<div class="navigator-back"><a href="/finanzen/' . $date . '"> &#8592; Back</a></div>';
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
    // dsm($category_id);
  	$spend = $form_state['values'][$first_day_month][$category_id]['spend'];
  	$reg_id = $form_state['values'][$first_day_month][$category_id]['id'];
  	
  	if ($reg_id != 0) { // DB Register already exists
	    $query = "UPDATE finanzen_outflow_sonstiges_month SET spend=" . $spend . 
                 " WHERE nid='" . $reg_id . "'";
        $queryResult = db_query ( $query );
    } 
    else { // DB Register does not exists
      if ($form_state['values'][$first_day_month][$category_id]['spend']!= 0) {
	    $query = "INSERT into {finanzen_outflow_sonstiges_month} ( category_id, date, spend )
                  values ( %d, '%s', %d )";
        $queryResult = db_query ( $query, $category_id, $first_day_month, $spend);   
      }
    }
  }
}

