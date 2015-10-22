// String utils

function escape_str ( str ){
     
    str = str.replace(/\\/g, "\\\\");
	str = str.replace(/\//g, "\\\/");
	
	return str;	
}