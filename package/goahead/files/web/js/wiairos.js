
function showMsg(title, msg, isAlert)
{
   if (isAlert !== undefined && isAlert) 
   {
       $.messager.alert(title, msg);
   }
   else 
   {
   		$.messager.show({
          	title: title,
   			msg: msg,
   			showType: 'show'
 		});
	}
}

function showConfirm(title, msg, callback)
{
	$.messager.confirm(title, msg, function (r)
	{
       if (r)
	   {
          if (jQuery.isFunction(callback))
                callback.call();
		}
    });
}

function showProcess(isShow, title, msg) 
{
	if (!isShow) 
	{
		$.messager.progress('close');
        return;
	}
    var win = $.messager.progress({
          title: title,
          msg: msg
     });
}