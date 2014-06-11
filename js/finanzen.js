/* Escribe aqui tu javascript, hipster developer :-) */

/* la próxima biblia se escribirá en JS nativo 

var dios  = new Dios(),
	world = dios.create('world');

world.addEventListener('seventh_day', function (e) {
	dios.descansa();

}, false);
// und so weiter und so fort...

*/

window.onload = function () {

	set_the_table_width('table.finanzen', '.navigator');
	center_month_name('div.navigator-date');
	
};

/* 
*	Align div which contains the menu and month name... with the width of the table
*	Arguments: query_name of the table, query_name of the div.navigator
*/

var set_the_table_width = function (table_name, navi_name) {
	
	try {
	
		//declaro variables de datos qeu necesito
		var doc     = document.body,
	
		table 	    = doc.querySelector(table_name),
		table_width = table.clientWidth,
		
		navi 		= doc.querySelector(navi_name);
		
		//hago la transición suave
		var declaration = 'all 500ms ease'
		navi.style.transition 		= declaration;
		navi.style.WebkitTransition = declaration;
		navi.style.MozTransition 	= declaration;
		
		//aplico el estilo
		navi.style.width = table_width +  'px';
	
	
	//por si da errores
	} catch (err) {
		
		var doc = document.body;
		
		if (doc.querySelector(table_name) == undefined) {
			throw 'I don\'t find the table';
		} else if (doc.querySelector(navi_name) == undefined) {
			throw 'I don\'t find the navigator';
		} else {
			throw 'Something went wrong and Javascript sucks...';
		}
	}
	
};

/* 
*	Center the name of the month within the navigation div
*	Arguments: query_name of the block with the name
*/

var center_month_name = function (block) {
	
	var doc 		= document.body,
	
		bl  		= doc.querySelector(block),
		bl_width 	= bl.clientWidth,
		
		navi 		= bl.parentNode,
		navi_width  = navi.clientWidth,
		
		tiny_navs   = doc.querySelector('.navigator-next'),
		tiny_navs_w = parseInt(tiny_navs.clientWidth),
		
		bl_position = ((navi.clientWidth / 2) - (bl.clientWidth / 2)) - (tiny_navs_w * 2);
	
	//positioning
	bl.style.position = 'relative';
	bl.style.left 	  = bl_position + 'px';
	
};

