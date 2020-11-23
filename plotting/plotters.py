import numpy as np

colors = ['lightgreen','lightsalmon','skyblue','steelblue','mediumseagreen','0.30','0.70','m']
hatches = ['/','\\\\','x','\\','//']

def plot_bars(ax,groups,labels,title,legend,to_compare,stdz,show_legend=False, on_top=False):

        N = len(labels)
        ind = np.arange(N)  # the x locations for the groups
        width = 0.15      # the width of the bars



        baseline = groups[0]

        # add some text for labels, title and axes ticks
        #ax.set_ylabel('Throughput (Mbps)')
        #ax.set_ylabel('Execution time (ms)')
	#ax.set_ylabel("Mops/sec")
        #ax.set_xlabel(title)
        #ax.set_xticks(ind + 2*width)
        ax.set_xticks(ind+1*width)
        ax.set_xticklabels(labels,rotation='0', fontsize=22)
        rects_set=[]
        lgd=[]
        c=0
        #bottoms = [0]*len(groups[0])
        bottoms = [0,0]
        for i,group in enumerate(groups):
                series = group
		print "WOW",series
		print c
                stand_dev = stdz[i]
		print stand_dev
                #rects = ax.bar(ind, series, 0.8*width , color=colors[c],hatch=hatches[c],yerr=stand_dev, ecolor='black',label=[legend[c]])
                #rects = ax.bar(ind, series, 1.0*width , color=colors[c],hatch=hatches[c],label=[legend[c]],bottom=bottoms, yerr=stand_dev,ecolor='black')
                #rects = ax.bar(ind, series, 1.0*width , color=colors[c],hatch=hatches[c], yerr=stand_dev,ecolor='black')
                rects = ax.bar(ind, series, 1.0*width  ,hatch=hatches[c], yerr=stand_dev,ecolor='black')
                v = [x.get_height() for x in rects]
                if (c in to_compare):
                        vals = np.divide(baseline,v)
                        #if switch_tp:
                        vals = np.divide(1,vals)
                        autolabel(ax,rects,vals,title,legend,c,labels)
                rects_set.append(rects)
                if not on_top:
                    ind = [x+width for x in ind]
                c+=1
                if on_top:
                    bottoms = list(np.array(bottoms)+ np.array(group))
        if show_legend:
            #ax.legend((rects_set),legend,loc=2,ncol=2)
            #ax.legend((rects_set),legend, loc=4,ncol=2)
            lgd = ax.legend((rects_set),legend,bbox_to_anchor=(0.,1.1,1.00,0.25),loc=2,ncol=2, mode="expand", borderaxespad=0.1,markerscale=12)
        
        ax.plot()
        return lgd

def autolabel(ax,rects,vals,title,legend,c,labels):
    # attach some text labels
    f=0
    for rect,val in zip(rects,vals):
        height = rect.get_height()
        if (title=="Snort web traffic patterns(2K)") and (legend[c]=="DFC") and (f==0)and (len(labels)!=1):
                ax.text(rect.get_x() + rect.get_width()/2., 0.3+height,
                '%0.2f' % (val),
                        ha='center', va='bottom')
        else:
                ax.text(rect.get_x() + rect.get_width()/2., 0.1+height,
                '%0.2f' % (val),
                        ha='center', va='bottom')

        f+=1

