<script type="text/javascript">
    $(document).ready(function() {

	var PhaseTurnNode = $("#navTree").omTree("getSelected").text;
    var NodeNo = PhaseTurnNode.substr(4);
    $("#FollowPhase").attr('value',NodeNo);

        var test = $("#reg").validate({
            rules : {
                GreenLight : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                RedLight : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                YellowLight : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    },
                GreenFlash : {
                      required:true,
                      min:0,
                      max:255,
                      number:true
                    }
                 },
                messages : {
                  GreenLight : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  RedLight : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  YellowLight : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    },
                  GreenFlash : {
                      required:"此处不能为空",
                      min:"取值范围为[0,255]",
                      max:"取值范围为[0,255]"
                    }
                },
                 submitHandler : function(){
                 return false;
                }
            });
        });
</script>
<form traffic='Overlapping' id="reg">
  	<br />
  	<h1>跟随相位</h1>
  	<br />
	<table border="1" cellspacing="0" cellpadding="0"  class="edittableNo">
		<tr>
			<td class="tdpaddingleft">跟随相位:</td>
			<td colspan="8" class="tdpaddingleft">
				<select name="FollowPhase" id="FollowPhase" class="selectwidth">
					<option value="1">1</option>
		            <option value="2">2</option>
		            <option value="3">3</option>
		            <option value="4">4</option>
		            <option value="5">5</option>
		            <option value="6">6</option>
		            <option value="7">7</option>
		            <option value="8">8</option>
		            <option value="9">9</option>
		            <option value="10">10</option>
		            <option value="11">11</option>
		            <option value="12">12</option>
		            <option value="13">13</option>
		            <option value="14">14</option>
		            <option value="15">15</option>
		            <option value="16">16</option>
		        </select>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" rowspan="5">时间(单位秒)&nbsp;&nbsp;</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="8">绿灯(/10):
				<input type="text" name="GreenLight" id="GreenLight"/>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="8">红灯(/10):
				<input type="text" name="RedLight" id="RedLight"/>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="8">黄灯(/10):
				<input type="text" name="YellowLight" id="YellowLight"/>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="8">绿闪(/10):
				<input type="text" name="GreenFlash" id="GreenFlash"/>
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" rowspan="4">母相位</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="ParentPhase1" id="ParentPhase1"/>1
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="ParentPhase2" id="ParentPhase2"/>2
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="ParentPhase3" id="ParentPhase3"/>3
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="ParentPhase4" id="ParentPhase4"/>4
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="ParentPhase5" id="ParentPhase5"/>5
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="ParentPhase6" id="ParentPhase6"/>6
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="ParentPhase7" id="ParentPhase7"/>7
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="ParentPhase8" id="ParentPhase8"/>8
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft">
				<input type="checkbox" name="ParentPhase9" id="ParentPhase9"/>9
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="ParentPhase10" id="ParentPhase10"/>10
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="ParentPhase11" id="ParentPhase11"/>11
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="ParentPhase12" id="ParentPhase12"/>12
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="ParentPhase13" id="ParentPhase13"/>13
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="ParentPhase14" id="ParentPhase14"/>14
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="ParentPhase15" id="ParentPhase15"/>15
			</td>
			<td class="tdpaddingleft">
				<input type="checkbox" name="ParentPhase16" id="ParentPhase16"/>16
			</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" rowspan="2">修正相位</td>
		</tr>
		<tr>
			<td class="tdpaddingleft" colspan="8">黄绿停止:
				<select name="ModifiedPhase" id="ModifiedPhase" class="selectwidth">
					<option value="1">无</option>
					<option value="2">1</option>
		            <option value="3">2</option>
		            <option value="4">3</option>
		            <option value="5">4</option>
		            <option value="6">5</option>
		            <option value="7">6</option>
		            <option value="8">7</option>
		            <option value="9">8</option>
		            <option value="10">9</option>
		        </select>
			</td>
		</tr>
	</table>
</form>
