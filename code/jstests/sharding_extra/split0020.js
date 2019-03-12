(function() {
    'use strict';
		//支持的特殊符号是！@#￥%…&*（）- — +=|{}[]:;'<>.?/(不支持\,")
		/*
		1、不可以含有保留字符$；
		2、不能是空字符串，如""；
		3、不能含有\0空字符；
		4、不能以"system."命名；
		5、database和集合名称总长度不超过120字节
		*/
		var special=["!","@","￥","%","+","*","[","#","a","1","&","-","=","|",":",";","<","?",">","}","{","(",")","]"]
		var rdm2=[];
		var str2="";
		var a=Math.floor(Math.random()*30);
		for (var j=0;j<=a;j++){
		var index=Math.floor(Math.random()*special.length);
		rdm2[j]=special[index];
		str2=str2+rdm2[j];
		}

		var rdm1=[];
		var a=Math.floor(Math.random()*20);
		var str1="";
		
		for (var j=0;j<=a;j++){
		var index=Math.floor(Math.random()*special.length);
		rdm1[j]=special.splice(index,1)[0];
		str1=str1+rdm1[j];
		}
		
		
		jsTest.log("--------------------------------------"+str1);
		jsTest.log("--------------------------------------"+str2);
		
        var st = new ShardingTest({shards: 3, mongos: 1});
		assert.commandWorked(st.startBalancer());
        var primarycs=st.configRS.getPrimary();var configSecondaryList = st.configRS.getSecondaries();var mgs=st.s0;
        var admin=mgs.getDB('admin');
        var cfg=mgs.getDB('config');
        var coll=mgs.getCollection(str2+"."+str1);
		var coll1=mgs.getCollection(str2+"."+str2);
        var testdb=mgs.getDB(str2);
        st.startBalancer();
        assert.commandWorked(admin.runCommand({enableSharding:str2}));
        assert.commandWorked(admin.runCommand({shardCollection:str2+"."+str1,key:{a:1}}));
        jsTest.log("-------------------insert data-------------------");
        for (var i=0;i<1000;i++){
			assert.writeOK(coll.insert({a: i, c: i}));}
        printShardingStatus(st.config,false);
        jsTest.log("-------------------splite-------------------");
		var ransp = Math.floor(Math.random()*1000);
        //assert.commandWorked(admin.runCommand({split: str2+"."+str1,manualsplit:true,find :{a : ransp}}));
		assert.commandWorked(admin.runCommand({split: str2+"."+str1,middle :{a : ransp}}));
        printShardingStatus(st.config,false);
		jsTest.log("-------------------confirm chunks normal-------------------");
        var chunks = cfg.chunks.find().toArray();
        var max = chunks[0].max.a;
		var min = chunks[1].min.a;
        assert.eq(max,ransp);
		assert.eq(min,ransp);
        var chunks = cfg.chunks.find().toArray();
        var num = cfg.chunks.find().itcount();
        assert.eq(num,2);
        jsTest.log("-------------------confirm update normal-------------------");
        var ransl = ransp - 100;
        var ransr = ransp + 100;
        var ranso = Math.floor(Math.random()*ransp);
        var num = 1000 - ransp ;
        var ransq = Math.floor(Math.random()*num + ransp);
        assert.writeOK(coll.insert({a: ransl,c: 1001}));
        assert.writeOK(coll.insert({a: ransr,c: 1002}));
        assert.commandWorked(admin.runCommand({shardCollection:str2+"."+str2,key:{b:1}}));
        assert.writeOK(coll1.insert({b: 10, d: 20}));
        assert.writeOK(coll.update({c: ranso},{$set : {c : 1003}}, false,true));
        assert.writeOK(coll.update({c: ransq},{$set : {c : 1004}}, false,true));
        assert.eq(1003,coll.find({a: ranso}).toArray()[0].c, "update  failed");
        assert.eq(1004,coll.find({a: ransq}).toArray()[0].c, "update  failed");
        st.stop();
})();
