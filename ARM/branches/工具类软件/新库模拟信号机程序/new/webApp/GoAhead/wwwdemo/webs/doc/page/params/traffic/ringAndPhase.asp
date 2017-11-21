<script type="text/javascript">
	var ActionMenu = new Array();
	var circleArray = new Array();
    var Hphase;
    var acctionID;

    for(Hphase=1;Hphase<=addPhase;Hphase++){
    	ActionMenu.push({id:"Hphase"+phaseArray[Hphase-1],text:phaseArray[Hphase-1],value:phaseArray[Hphase-1]});
    }
	
	for(var i = 1 ; i <= 4 ; i++)
	{
		circleArray.push({id:"circle"+i,text:i,value:i});
	}

    for(Hphase=1;Hphase<=addPhase;Hphase++)
    {
    	acctionID = "SamePhase"+phaseArray[Hphase-1];//并发相位ID
    	var x=document.getElementById('ConcurrentPhaseTable').insertRow(Hphase);
		var y=x.insertCell(0);
		var z=x.insertCell(1);
		var w=x.insertCell(2);
		y.innerHTML="相位"+phaseArray[Hphase-1];
		var RingForPhase = "RingForPhase"+phaseArray[Hphase-1];
		//z.innerHTML="<select name="+RingForPhase+" id="+RingForPhase+" class='selectwidth' ><option value='1'>1</option><option value='2'>2</option><option //value='3'>3</option><option value='4'>4</option></select>";
		z.innerHTML = "<input type='text' id="+RingForPhase+ " value=1>";
		w.innerHTML = "<input type='text' id="+acctionID+ " value=' '>";
		
		//if(Hphase == 1)
		//	alert(document.getElementById("RingForPhase1").disabled);
		//document.getElementById("RingForPhase1").disabled = true;
		$(document).ready(function() {
            $('#SamePhase'+phaseArray[Hphase-1]).omCombo({
                dataSource : ActionMenu,
                multi : true,
				multiSeparator : ',',
				//disabled:true,
				forceSelection: true
			});
			$('#'+RingForPhase).omCombo({
				dataSource: circleArray,
				disabled:true
			});

		});

    }
	$("#TotalPhaseCount").text(document.getElementById('ConcurrentPhaseTable').rows.length - 1);//这里有个隐藏字段用来传递相位总数，方便BOARD轮询。

	//alert($("#TotalPhaseCount").text());
</script>
<form traffic='ringAndphase'>
    <br />
    <h1>环/并发相位  <font color="red">请注意，修改完本项目后请点击"手动保存当前参数"！</font></h1>
    <br />
	<table border="1" cellspacing="0" cellpadding="0" class="edittableNo" id="ConcurrentPhaseTable">
		<tr>
			<td class="tdpaddingleft" id = "RingForPhase">相位</td>
			<td class="tdpaddingleft">属于环</td>
			<td class="tdpaddingleft">并发相位</td>
		</tr>
	</table>
	<p id="TotalPhaseCount"  style="display:none" >相位总数</p>
</form>

